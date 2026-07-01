#include "paging.h"
#include "pmm.h"
#include "../drivers/vga.h"

/* Page directory — 1024 entries, each points to a page table */
static uint32_t page_directory[1024] __attribute__((aligned(4096)));

/* Page table for first 4MB (identity mapped) */
static uint32_t page_table_0[1024] __attribute__((aligned(4096)));

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

void paging_init() {
    /* Clear page directory — mark all entries not present */
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;  /* not present, writable */
    }

    /* Identity map first 4MB:
       virtual 0x00000000 - 0x003FFFFF → same physical addresses
       This keeps our kernel code/data working after paging is enabled */
    for (int i = 0; i < 1024; i++) {
        page_table_0[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;
    }

    /* Put page_table_0 into slot 0 of page directory */
    page_directory[0] = (uint32_t)page_table_0 | PAGE_PRESENT | PAGE_WRITABLE;

    /* Also map kernel at 1MB+ — our kernel lives at 0x100000
       which is already covered by the identity map above (first 4MB) */

    /* Load page directory into CR3 */
    __asm__ volatile("mov %0, %%cr3" : : "r"((uint32_t)page_directory));

    /* Enable paging by setting bit 31 of CR0 */
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));

    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("Paging enabled | Identity mapped: 0x0 - ");
    print_hex(4 * 1024 * 1024);
    vga_print("\n");
}

void map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t pd_idx = virt >> 22;          /* Top 10 bits = page directory index */
    uint32_t pt_idx = (virt >> 12) & 0x3FF; /* Next 10 bits = page table index */

    /* If page table doesn't exist, allocate one */
    if (!(page_directory[pd_idx] & PAGE_PRESENT)) {
        uint32_t* new_table = (uint32_t*)pmm_alloc_page();
        if (!new_table) return;  /* Out of memory */

        /* Zero the new page table */
        for (int i = 0; i < 1024; i++) new_table[i] = 0;

        page_directory[pd_idx] = (uint32_t)new_table | PAGE_PRESENT | PAGE_WRITABLE | (flags & PAGE_USER);
    }

    /* Get pointer to page table */
    uint32_t* page_table = (uint32_t*)(page_directory[pd_idx] & ~0xFFF);

    /* Set the page table entry */
    page_table[pt_idx] = (phys & ~0xFFF) | (flags & 0xFFF) | PAGE_PRESENT;

    /* Flush TLB for this address */
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void unmap_page(uint32_t virt) {
    uint32_t pd_idx = virt >> 22;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;

    if (!(page_directory[pd_idx] & PAGE_PRESENT)) return;

    uint32_t* page_table = (uint32_t*)(page_directory[pd_idx] & ~0xFFF);
    page_table[pt_idx] = 0;

    /* Flush TLB */
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

uint32_t get_physical(uint32_t virt) {
    uint32_t pd_idx = virt >> 22;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;

    if (!(page_directory[pd_idx] & PAGE_PRESENT)) return 0;

    uint32_t* page_table = (uint32_t*)(page_directory[pd_idx] & ~0xFFF);
    if (!(page_table[pt_idx] & PAGE_PRESENT)) return 0;

    return (page_table[pt_idx] & ~0xFFF) | (virt & 0xFFF);
}