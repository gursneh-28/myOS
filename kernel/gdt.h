#ifndef GDT_H
#define GDT_H

#include <stdint.h>

/* GDT Entry structure - each entry is 8 bytes */
struct gdt_entry {
    uint16_t limit_low;     /* Lower 16 bits of limit */
    uint16_t base_low;      /* Lower 16 bits of base */
    uint8_t  base_middle;   /* Next 8 bits of base */
    uint8_t  access;        /* Access flags */
    uint8_t  granularity;   /* Granularity flags */
    uint8_t  base_high;     /* Last 8 bits of base */
} __attribute__((packed));

/* Pointer to GDT - what we load with lgdt instruction */
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void gdt_init();

#endif