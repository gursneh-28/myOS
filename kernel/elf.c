#include "elf.h"
#include "fs.h"
#include "paging.h"
#include "../drivers/vga.h"

static void print_hex(uint32_t val) {
    char buf[11];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 9; i >= 2; i--) {
        int n = val & 0xF;
        buf[i] = n < 10 ? '0' + n : 'a' + n - 10;
        val >>= 4;
    }
    buf[10] = '\0';
    vga_print(buf);
}

static void mem_copy(uint8_t* dst, const uint8_t* src, uint32_t n) {
    for (uint32_t i = 0; i < n; i++) dst[i] = src[i];
}

static void mem_zero(uint8_t* dst, uint32_t n) {
    for (uint32_t i = 0; i < n; i++) dst[i] = 0;
}

int elf_load(const char* filename) {
    /* Find file in filesystem */
    struct file* f = fs_open(filename);
    if (!f) {
        vga_print_color("\n[ELF] File not found: ", COLOR_RED, COLOR_BLACK);
        vga_print(filename);
        vga_print("\n");
        return -1;
    }

    uint8_t* data = (uint8_t*)f->data;

    /* Validate ELF header */
    struct elf_header* hdr = (struct elf_header*)data;
    if (hdr->magic != ELF_MAGIC) {
        vga_print_color("\n[ELF] Invalid ELF magic\n", COLOR_RED, COLOR_BLACK);
        return -1;
    }
    if (hdr->bits != 1) {
        vga_print_color("\n[ELF] Not a 32-bit ELF\n", COLOR_RED, COLOR_BLACK);
        return -1;
    }
    if (hdr->type != ET_EXEC) {
        vga_print_color("\n[ELF] Not an executable\n", COLOR_RED, COLOR_BLACK);
        return -1;
    }

    vga_print_color("\n[ELF] Loading: ", COLOR_CYAN, COLOR_BLACK);
    vga_print(filename);
    vga_print(" | Entry: ");
    print_hex(hdr->entry);
    vga_print("\n");

    /* Load each PT_LOAD segment */
    for (int i = 0; i < hdr->ph_count; i++) {
        struct elf_program_header* ph = (struct elf_program_header*)
            (data + hdr->ph_offset + i * hdr->ph_entry_size);

        if (ph->type != PT_LOAD) continue;

        vga_print_color("[ELF] Segment: ", COLOR_CYAN, COLOR_BLACK);
        vga_print("vaddr="); print_hex(ph->vaddr);
        vga_print(" size="); print_hex(ph->mem_size);
        vga_print("\n");

        /* Map pages for this segment */
        uint32_t vaddr_start = ph->vaddr & ~0xFFF;
        uint32_t vaddr_end   = ph->vaddr + ph->mem_size;
        for (uint32_t addr = vaddr_start; addr < vaddr_end; addr += 4096) {
            /* Use the address directly since we're identity mapped */
            map_page(addr, addr, 0x7);  /* present | writable | user */
        }

        /* Copy file data to virtual address */
        mem_copy((uint8_t*)ph->vaddr, data + ph->offset, ph->file_size);

        /* Zero the BSS (mem_size > file_size) */
        if (ph->mem_size > ph->file_size) {
            mem_zero((uint8_t*)(ph->vaddr + ph->file_size),
                     ph->mem_size - ph->file_size);
        }
    }

    /* Jump to entry point */
    vga_print_color("[ELF] Jumping to entry point...\n", COLOR_CYAN, COLOR_BLACK);

    typedef void (*entry_fn)();
    entry_fn entry = (entry_fn)hdr->entry;
    entry();

    vga_print_color("[ELF] Program exited\n", COLOR_GREEN, COLOR_BLACK);
    return 0;
}