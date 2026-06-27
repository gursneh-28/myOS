#ifndef VGA_H
#define VGA_H

/* VGA Colors */
typedef enum {
    COLOR_BLACK   = 0,
    COLOR_BLUE    = 1,
    COLOR_GREEN   = 2,
    COLOR_CYAN    = 3,
    COLOR_RED     = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN   = 6,
    COLOR_GREY    = 7,
    COLOR_WHITE   = 15,
} vga_color;

void vga_init();
void vga_putchar(char c);
void vga_backspace();        /* ← add this */
void vga_print(const char* str);
void vga_print_color(const char* str, vga_color fg, vga_color bg);
void vga_clear();
void vga_set_color(vga_color fg, vga_color bg);

#endif