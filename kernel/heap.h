#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>

void heap_init();
void* kmalloc(uint32_t size);
void kfree(void* ptr);
void heap_dump();  /* debug: print heap state */

#endif