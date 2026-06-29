#include "shell.h"
#include "../drivers/vga.h"
#include "../kernel/heap.h"
#include "../kernel/pmm.h"
#include "../drivers/timer.h"

#define INPUT_BUFFER_SIZE 256

static char input_buffer[INPUT_BUFFER_SIZE];
static int input_len = 0;

static void print_int(uint32_t val) {
    if (val == 0) { vga_print("0"); return; }
    char buf[12];
    int i = 11;
    buf[i] = '\0';
    while (val > 0) {
        buf[--i] = '0' + (val % 10);
        val /= 10;
    }
    vga_print(&buf[i]);
}

static void shell_prompt() {
    vga_print_color("\n> ", COLOR_GREEN, COLOR_BLACK);
}

static int str_equals(const char* a, const char* b) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return a[i] == '\0' && b[i] == '\0';
}

static void cmd_help() {
    vga_print_color("\nAvailable commands:\n", COLOR_CYAN, COLOR_BLACK);
    vga_print_color("  help     ", COLOR_GREEN, COLOR_BLACK); vga_print("- Show this help message\n");
    vga_print_color("  clear    ", COLOR_GREEN, COLOR_BLACK); vga_print("- Clear the screen\n");
    vga_print_color("  about    ", COLOR_GREEN, COLOR_BLACK); vga_print("- About myOS\n");
    vga_print_color("  echo     ", COLOR_GREEN, COLOR_BLACK); vga_print("- Echo text (e.g. echo hello)\n");
    vga_print_color("  meminfo  ", COLOR_GREEN, COLOR_BLACK); vga_print("- Show memory usage\n");
    vga_print_color("  uptime   ", COLOR_GREEN, COLOR_BLACK); vga_print("- Show time since boot\n");
    vga_print_color("  version  ", COLOR_GREEN, COLOR_BLACK); vga_print("- Show OS version info\n");
    vga_print_color("  reboot   ", COLOR_GREEN, COLOR_BLACK); vga_print("- Reboot the system\n");
    vga_print_color("  shutdown ", COLOR_GREEN, COLOR_BLACK); vga_print("- Power off the system\n");
}

static void cmd_clear() {
    vga_clear();
}

static void cmd_about() {
    vga_print_color("\n  myOS v0.2\n", COLOR_CYAN, COLOR_BLACK);
    vga_print("  A simple x86 OS built from scratch\n");
    vga_print("  Arch    : x86 32-bit\n");
    vga_print("  Boot    : GRUB Multiboot\n");
    vga_print("  Features: VGA, GDT, IDT, PIC, IRQ,\n");
    vga_print("            Keyboard, PMM, Heap, Timer\n");
}

static void cmd_echo(const char* args) {
    vga_print("\n");
    vga_print(args);
    vga_print("\n");
}

static void cmd_meminfo() {
    vga_print_color("\n--- Memory Info ---\n", COLOR_CYAN, COLOR_BLACK);

    uint32_t free_p  = pmm_get_free_pages();
    uint32_t total_p = pmm_get_total_pages();
    uint32_t used_p  = total_p - free_p;

    vga_print("  Physical Memory:\n");
    vga_print("    Total : "); print_int(total_p * 4); vga_print(" KB (");
    print_int(total_p * 4 / 1024); vga_print(" MB)\n");
    vga_print("    Used  : "); print_int(used_p * 4);  vga_print(" KB\n");
    vga_print("    Free  : "); print_int(free_p * 4);  vga_print(" KB (");
    print_int(free_p * 4 / 1024); vga_print(" MB)\n");

    vga_print("\n  Heap:\n");
    heap_dump();
}

static void cmd_uptime() {
    uint32_t seconds = timer_get_seconds();
    uint32_t minutes = seconds / 60;
    uint32_t hours   = minutes / 60;
    seconds %= 60;
    minutes %= 60;

    vga_print("\n  Uptime: ");
    print_int(hours);   vga_print("h ");
    print_int(minutes); vga_print("m ");
    print_int(seconds); vga_print("s\n");
}

static void cmd_version() {
    vga_print_color("\n  myOS\n", COLOR_CYAN, COLOR_BLACK);
    vga_print("  Version  : 0.2\n");
    vga_print("  Arch     : x86 32-bit protected mode\n");
    vga_print("  Compiler : GCC (freestanding)\n");
    vga_print("  Assembler: NASM\n");
    vga_print("  Boot     : GRUB2 Multiboot\n");
    vga_print("  Author   : Gursneh\n");
}

static void cmd_reboot() {
    vga_print_color("\nRebooting...\n", COLOR_RED, COLOR_BLACK);
    /* Pulse CPU reset line via keyboard controller */
    uint8_t val = 0;
    while (val & 0x02) {
        __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"((uint16_t)0x64));
    }
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0xFE), "Nd"((uint16_t)0x64));
    __asm__ volatile("hlt");
}

static void cmd_shutdown() {
    vga_print_color("\nShutting down myOS...\n", COLOR_RED, COLOR_BLACK);
    __asm__ volatile("outw %0, %1" : : "a"((unsigned short)0x2000), "Nd"((unsigned short)0x604));
}

static void shell_execute() {
    input_buffer[input_len] = '\0';

    if (input_len == 0) {
        shell_prompt();
        return;
    }

    int cmd_end = 0;
    while (cmd_end < input_len && input_buffer[cmd_end] != ' ') cmd_end++;

    char saved = input_buffer[cmd_end];
    input_buffer[cmd_end] = '\0';
    const char* cmd  = input_buffer;
    const char* args = (saved == ' ') ? &input_buffer[cmd_end + 1] : "";

    if      (str_equals(cmd, "help"))     cmd_help();
    else if (str_equals(cmd, "clear"))    cmd_clear();
    else if (str_equals(cmd, "about"))    cmd_about();
    else if (str_equals(cmd, "echo"))     cmd_echo(args);
    else if (str_equals(cmd, "meminfo"))  cmd_meminfo();
    else if (str_equals(cmd, "uptime"))   cmd_uptime();
    else if (str_equals(cmd, "version"))  cmd_version();
    else if (str_equals(cmd, "reboot"))   cmd_reboot();
    else if (str_equals(cmd, "shutdown")) cmd_shutdown();
    else {
        vga_print_color("\nUnknown command: ", COLOR_RED, COLOR_BLACK);
        vga_print(cmd);
        vga_print("\nType 'help' for available commands.\n");
    }

    input_buffer[cmd_end] = saved;
    input_len = 0;
    shell_prompt();
}

void shell_handle_char(char c) {
    if (c == '\n') {
        shell_execute();
    } else if (c == '\b') {
        if (input_len > 0) {
            input_len--;
            vga_backspace();
        }
    } else {
        if (input_len < INPUT_BUFFER_SIZE - 1) {
            input_buffer[input_len++] = c;
            char str[2] = {c, '\0'};
            vga_print_color(str, COLOR_GREEN, COLOR_BLACK);
        }
    }
}

void shell_init() {
    input_len = 0;
    shell_prompt();
}