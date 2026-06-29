#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "shell.h"
#include "pmm.h"
#include "heap.h"

void kernel_main(unsigned int* mboot_ptr) {
    vga_init();

    vga_print_color("========================================\n", COLOR_CYAN, COLOR_BLACK);
    vga_print_color("          Welcome to myOS v0.2          \n", COLOR_CYAN, COLOR_BLACK);
    vga_print_color("========================================\n", COLOR_CYAN, COLOR_BLACK);

    vga_print_color("\n[BOOT] ", COLOR_CYAN, COLOR_BLACK);
    vga_print("Initializing GDT...\n");
    gdt_init();
    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("GDT loaded\n");

    vga_print_color("[BOOT] ", COLOR_CYAN, COLOR_BLACK);
    vga_print("Initializing IDT...\n");
    idt_init();
    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("IDT loaded\n");

    vga_print_color("[BOOT] ", COLOR_CYAN, COLOR_BLACK);
    vga_print("Initializing PMM...\n");
    pmm_init(mboot_ptr);

    vga_print_color("[BOOT] ", COLOR_CYAN, COLOR_BLACK);
    vga_print("Initializing Heap...\n");
    heap_init();

    keyboard_init();

    __asm__ volatile("sti");
    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("Interrupts enabled\n");

    vga_print_color("\n========================================\n", COLOR_CYAN, COLOR_BLACK);
    vga_print_color("       myOS ready! Type 'help'          \n", COLOR_WHITE, COLOR_BLACK);
    vga_print_color("========================================\n", COLOR_CYAN, COLOR_BLACK);

    shell_init();
}