#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* IDT Entry - each entry is 8 bytes */
struct idt_entry {
    uint16_t base_low;      /* Lower 16 bits of handler address */
    uint16_t selector;      /* Kernel code segment selector */
    uint8_t  zero;          /* Always zero */
    uint8_t  flags;         /* Flags */
    uint16_t base_high;     /* Upper 16 bits of handler address */
} __attribute__((packed));

/* Pointer to IDT */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* Registers pushed by our ISR */
struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pusha order (reversed) */
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

void idt_init();
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif