#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

/* Page Directory/Table Entry flags */
#define PAGE_PRESENT    0x1
#define PAGE_WRITABLE   0x2
#define PAGE_USER       0x4

void paging_init();
void map_page(uint32_t virt, uint32_t phys, uint32_t flags);
void unmap_page(uint32_t virt);
uint32_t get_physical(uint32_t virt);

#endif