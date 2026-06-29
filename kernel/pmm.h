#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096

void pmm_init(unsigned int* mboot_ptr);
void* pmm_alloc_page();
void pmm_free_page(void* ptr);
uint32_t pmm_get_free_pages();
uint32_t pmm_get_total_pages();

#endif