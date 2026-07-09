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

static char scancode_map_shift[] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*',
    '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"', '~', 0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N',
    'M', '<', '>', '?',  0,   '*', 0,   ' '
};

#define SC_LSHIFT      0x2A
#define SC_RSHIFT      0x36
#define SC_LSHIFT_REL  0xAA
#define SC_RSHIFT_REL  0xB6
#define SC_CAPS        0x3A
#define SC_BACKSPACE   0x0E
#define SC_EXTENDED    0xE0
#define SC_UP_ARROW    0x48
#define SC_DOWN_ARROW  0x50

static uint8_t shift_pressed = 0;
static uint8_t caps_lock     = 0;
static uint8_t extended      = 0;  /* set when 0xE0 prefix received */

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static char apply_case(char c) {
    if (c >= 'a' && c <= 'z') {
        if (caps_lock ^ shift_pressed) return c - 32;
        return c;
    }
    return c;
}

static void keyboard_handler(struct registers* r) {
    uint8_t scancode = inb(0x60);

    /* Extended key prefix */
    if (scancode == SC_EXTENDED) {
        extended = 1;
        return;
    }

    /* Handle extended keys */
    if (extended) {
        extended = 0;
        if (scancode == SC_UP_ARROW) {
            shell_history_up();
            return;
        }
        if (scancode == SC_DOWN_ARROW) {
            shell_history_down();
            return;
        }
        return;  /* ignore other extended keys */
    }

    /* Shift release */
    if (scancode == SC_LSHIFT_REL || scancode == SC_RSHIFT_REL) {
        shift_pressed = 0;
        return;
    }

    if (scancode & 0x80) return;  /* ignore other releases */

    /* Modifiers */
    if (scancode == SC_LSHIFT || scancode == SC_RSHIFT) {
        shift_pressed = 1;
        return;
    }
    if (scancode == SC_CAPS) {
        caps_lock ^= 1;
        return;
    }
    if (scancode == SC_BACKSPACE) {
        shell_handle_char('\b');
        return;
    }

    if (scancode < sizeof(scancode_map)) {
        char c;
        if (shift_pressed) {
            c = scancode_map_shift[scancode];
        } else {
            c = scancode_map[scancode];
            c = apply_case(c);
        }
        if (c) shell_handle_char(c);
    }
}

void keyboard_init() {
    shift_pressed = 0;
    caps_lock     = 0;
    extended      = 0;
    irq_install_handler(1, keyboard_handler);
    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("Keyboard driver initialized\n");
}