#include "isr.h"
#include "syscall.h"
#include "../drivers/vga.h"

/* Array of IRQ handlers */
static irq_handler_t irq_handlers[16] = {0};

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void isr_handler(struct registers* r) {
    if (r->int_no == 128) {
        syscall_dispatch(r);
        return;
    }

    vga_print_color("\n[EXCEPTION] ", COLOR_RED, COLOR_BLACK);
    if (r->int_no < 16) {
        vga_print(exception_names[r->int_no]);
    }
    vga_print("\nSystem Halted!\n");
    __asm__ volatile("cli; hlt");
}

void irq_install_handler(int irq, irq_handler_t handler) {
    irq_handlers[irq] = handler;
}

void irq_uninstall_handler(int irq) {
    irq_handlers[irq] = 0;
}

void irq_handler(struct registers* r) {
    if (r->int_no >= 40) outb(0xA0, 0x20);
    outb(0x20, 0x20);

    if (r->int_no >= 32 && r->int_no < 48) {
        irq_handler_t handler = irq_handlers[r->int_no - 32];
        if (handler) handler(r);
    }
}

void pic_remap() {
    uint8_t mask1 = inb(0x21);
    uint8_t mask2 = inb(0xA1);

    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, mask1);
    outb(0xA1, mask2);
}