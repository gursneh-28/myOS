#include "../drivers/vga.h"

void kernel_main() {
    vga_init();

    /* Print banner in cyan */
    vga_print_color("========================================\n", COLOR_CYAN, COLOR_BLACK);
    vga_print_color("          Welcome to myOS v0.1          \n", COLOR_CYAN, COLOR_BLACK);
    vga_print_color("========================================\n", COLOR_CYAN, COLOR_BLACK);

    /* System info in white */
    vga_print_color("\n[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("Kernel loaded successfully\n");

    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("VGA driver initialized\n");

    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("32-bit Protected Mode active\n");

    /* Warning example */
    vga_print_color("\n[!!] ", COLOR_RED, COLOR_BLACK);
    vga_print_color("This is myOS - a custom kernel from scratch!\n", COLOR_WHITE, COLOR_BLACK);

    vga_print_color("\nReady.\n", COLOR_GREY, COLOR_BLACK);
}