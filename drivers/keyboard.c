#include "keyboard.h"
#include "vga.h"
#include "../kernel/shell.h"

static char scancode_map[] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,  '\\', 'z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/',  0,   '*', 0,   ' '
};

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void keyboard_handler(struct registers* r) {
    uint8_t scancode = inb(0x60);

    if (scancode & 0x80) return;

    if (scancode == 0x0E) {
        shell_handle_char('\b');
        return;
    }

    if (scancode < sizeof(scancode_map)) {
        char c = scancode_map[scancode];
        if (c) shell_handle_char(c);
    }
}

void keyboard_init() {
    irq_install_handler(1, keyboard_handler);
    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("Keyboard driver initialized\n");
}