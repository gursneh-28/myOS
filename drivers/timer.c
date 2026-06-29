#include "timer.h"
#include "../kernel/isr.h"
#include "../drivers/vga.h"

static uint32_t ticks = 0;
static uint32_t frequency = 0;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void timer_handler(struct registers* r) {
    ticks++;
}

void timer_init(uint32_t freq) {
    frequency = freq;
    irq_install_handler(0, timer_handler);

    /* PIT frequency = 1193180 Hz / divisor */
    uint32_t divisor = 1193180 / freq;
    outb(0x43, 0x36);                    /* Command: channel 0, square wave */
    outb(0x40, divisor & 0xFF);          /* Low byte */
    outb(0x40, (divisor >> 8) & 0xFF);  /* High byte */

    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("Timer initialized at ");
    /* print frequency */
    char buf[12];
    int i = 11;
    buf[i] = '\0';
    uint32_t v = freq;
    if (v == 0) { buf[--i] = '0'; }
    while (v > 0) { buf[--i] = '0' + (v % 10); v /= 10; }
    vga_print(&buf[i]);
    vga_print(" Hz\n");
}

uint32_t timer_get_ticks() {
    return ticks;
}

uint32_t timer_get_seconds() {
    if (frequency == 0) return 0;
    return ticks / frequency;
}