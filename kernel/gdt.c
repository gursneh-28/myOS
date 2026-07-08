#include "gdt.h"

#define GDT_ENTRIES 6   /* null, kernel code, kernel data, user code, user data, TSS */

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr   gp;
static struct tss_entry tss;

extern void gdt_flush(uint32_t);
extern void tss_flush();

static void gdt_set_entry(int num, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

static void tss_init(uint32_t kernel_stack) {
    uint32_t base  = (uint32_t)&tss;
    uint32_t limit = base + sizeof(struct tss_entry);

    /* Add TSS descriptor to GDT slot 5 */
    gdt_set_entry(5, base, limit, 0x89, 0x00);

    /* Clear TSS */
    for (uint32_t i = 0; i < sizeof(struct tss_entry); i++)
        ((uint8_t*)&tss)[i] = 0;

    tss.ss0  = 0x10;           /* Kernel data segment */
    tss.esp0 = kernel_stack;   /* Kernel stack */
    tss.cs   = 0x08 | 0x3;    /* User code segment (ring 3) */
    tss.ss   = tss.ds = tss.es = tss.fs = tss.gs = 0x10 | 0x3;
    tss.iomap_base = sizeof(struct tss_entry);
}

void tss_set_kernel_stack(uint32_t stack) {
    tss.esp0 = stack;
}

void gdt_init() {
    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base  = (uint32_t)&gdt;

    gdt_set_entry(0, 0, 0,          0x00, 0x00); /* Null */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* Kernel code  (ring 0) */
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* Kernel data  (ring 0) */
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* User code    (ring 3) */
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* User data    (ring 3) */
    tss_init(0);  /* kernel stack set later */

    gdt_flush((uint32_t)&gp);
    tss_flush();
}