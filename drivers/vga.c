#include "vga.h"

#define VGA_ADDRESS  0xB8000
#define VGA_WIDTH    80
#define VGA_HEIGHT   25

static unsigned short* vga = (unsigned short*)VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;
static unsigned char current_color = 0x0F; /* white on black */

static unsigned char make_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

static unsigned short make_entry(char c, unsigned char color) {
    return (unsigned short)c | (unsigned short)color << 8;
}

static void scroll() {
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga[y * VGA_WIDTH + x] = vga[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = make_entry(' ', current_color);
    }
    cursor_y = VGA_HEIGHT - 1;
}

void vga_clear() {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga[y * VGA_WIDTH + x] = make_entry(' ', current_color);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

void vga_set_color(vga_color fg, vga_color bg) {
    current_color = make_color(fg, bg);
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~3;
    } else {
        vga[cursor_y * VGA_WIDTH + cursor_x] = make_entry(c, current_color);
        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= VGA_HEIGHT) {
        scroll();
    }
}

void vga_backspace() {
    if (cursor_x > 0) {
        cursor_x--;
    } else if (cursor_y > 0) {
        /* Go to end of previous line */
        cursor_y--;
        cursor_x = VGA_WIDTH - 1;
        /* Scan back to find last non-space character */
        while (cursor_x > 0) {
            unsigned short entry = vga[cursor_y * VGA_WIDTH + cursor_x];
            char c = entry & 0xFF;
            if (c != ' ') break;
            cursor_x--;
        }
        cursor_x++; /* Position after last character */
        if (cursor_x >= VGA_WIDTH) cursor_x = VGA_WIDTH - 1;
    }
    /* Erase the character at cursor */
    vga[cursor_y * VGA_WIDTH + cursor_x] = make_entry(' ', current_color);
}

void vga_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_putchar(str[i]);
    }
}

void vga_print_color(const char* str, vga_color fg, vga_color bg) {
    unsigned char old_color = current_color;
    vga_set_color(fg, bg);
    vga_print(str);
    current_color = old_color;
}

void vga_init() {
    vga_set_color(COLOR_WHITE, COLOR_BLACK);
    vga_clear();
}