#include "pmm.h"
#include "../drivers/vga.h"

/* Multiboot memory map entry */
struct mmap_entry {
    uint32_t size;
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t len_low;
    uint32_t len_high;
    uint32_t type;      /* 1 = available, anything else = reserved */
} __attribute__((packed));

/* Multiboot info structure (partial) */
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;   /* KB of lower memory */
    uint32_t mem_upper;   /* KB of upper memory */
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} __attribute__((packed));

#define MAX_PAGES 32768         /* Support up to 128MB (32768 * 4KB) */
#define BITMAP_SIZE (MAX_PAGES / 32)

static uint32_t bitmap[BITMAP_SIZE];
static uint32_t total_pages = 0;
static uint32_t free_pages  = 0;

/* Kernel end symbol from linker */
extern uint32_t _kernel_end;

static void bitmap_set(uint32_t page) {
    bitmap[page / 32] |= (1 << (page % 32));
}

static void bitmap_clear(uint32_t page) {
    bitmap[page / 32] &= ~(1 << (page % 32));
}

static int bitmap_test(uint32_t page) {
    return bitmap[page / 32] & (1 << (page % 32));
}

static void print_hex(uint32_t val) {
    char buf[11];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 9; i >= 2; i--) {
        int nibble = val & 0xF;
        buf[i] = nibble < 10 ? '0' + nibble : 'a' + nibble - 10;
        val >>= 4;
    }
    buf[10] = '\0';
    vga_print(buf);
}

static void print_int(uint32_t val) {
    if (val == 0) { vga_print("0"); return; }
    char buf[12];
    int i = 11;
    buf[i] = '\0';
    while (val > 0) {
        buf[--i] = '0' + (val % 10);
        val /= 10;
    }
    vga_print(&buf[i]);
}

void pmm_init(unsigned int* mboot_ptr) {
    struct multiboot_info* mbi = (struct multiboot_info*)mboot_ptr;

    /* Mark all pages as used initially */
    for (int i = 0; i < BITMAP_SIZE; i++) {
        bitmap[i] = 0xFFFFFFFF;
    }

    /* Use mmap if available (flag bit 6) */
    if (mbi->flags & (1 << 6)) {
        struct mmap_entry* entry = (struct mmap_entry*)mbi->mmap_addr;
        uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

        while ((uint32_t)entry < mmap_end) {
            /* Only handle 32-bit addresses, type 1 = available */
            if (entry->addr_high == 0 && entry->type == 1) {
                uint32_t start = entry->addr_low;
                uint32_t len   = entry->len_low;
                uint32_t end   = start + len;

                uint32_t page_start = (start + PAGE_SIZE - 1) / PAGE_SIZE;
                uint32_t page_end   = end / PAGE_SIZE;

                if (page_end > MAX_PAGES) page_end = MAX_PAGES;

                for (uint32_t p = page_start; p < page_end; p++) {
                    bitmap_clear(p);
                    free_pages++;
                    total_pages++;
                }
            }
            entry = (struct mmap_entry*)((uint32_t)entry + entry->size + 4);
        }
    } else {
        /* Fallback: use mem_upper from multiboot */
        uint32_t mem_kb = mbi->mem_upper;
        uint32_t pages  = (mem_kb * 1024) / PAGE_SIZE;
        if (pages > MAX_PAGES) pages = MAX_PAGES;
        for (uint32_t p = 256; p < pages; p++) {  /* skip first 1MB */
            bitmap_clear(p);
            free_pages++;
            total_pages++;
        }
    }

    /* Mark page 0 as used (null pointer protection) */
    bitmap_set(0);

    /* Mark kernel pages as used */
    uint32_t kernel_end = (uint32_t)&_kernel_end;
    uint32_t kernel_pages = (kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t p = 0; p < kernel_pages && p < MAX_PAGES; p++) {
        if (!bitmap_test(p)) {
            bitmap_set(p);
            free_pages--;
        }
    }

    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("PMM initialized | Free: ");
    print_int(free_pages);
    vga_print(" pages (");
    print_int((free_pages * PAGE_SIZE) / 1024 / 1024);
    vga_print(" MB) | Total: ");
    print_int((total_pages * PAGE_SIZE) / 1024 / 1024);
    vga_print(" MB\n");
}

void* pmm_alloc_page() {
    for (uint32_t i = 0; i < MAX_PAGES; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            free_pages--;
            return (void*)(i * PAGE_SIZE);
        }
    }
    return 0;  /* Out of memory */
}

void pmm_free_page(void* ptr) {
    uint32_t page = (uint32_t)ptr / PAGE_SIZE;
    if (page < MAX_PAGES && bitmap_test(page)) {
        bitmap_clear(page);
        free_pages++;
    }
}

uint32_t pmm_get_free_pages()  { return free_pages; }
uint32_t pmm_get_total_pages() { return total_pages; }