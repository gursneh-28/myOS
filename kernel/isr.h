#ifndef ISR_H
#define ISR_H

#include "idt.h"

/* Exception names */
static const char *exception_names[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt"
};

typedef void (*irq_handler_t)(struct registers*);

void isr_handler(struct registers* r);
void irq_handler(struct registers* r);
void irq_install_handler(int irq, irq_handler_t handler);
void irq_uninstall_handler(int irq);
void pic_remap();
#endif