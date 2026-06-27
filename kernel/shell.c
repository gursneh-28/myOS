#include "shell.h"
#include "../drivers/vga.h"

#define INPUT_BUFFER_SIZE 256

static char input_buffer[INPUT_BUFFER_SIZE];
static int input_len = 0;

static void shell_prompt() {
    vga_print_color("\n> ", COLOR_GREEN, COLOR_BLACK);
}

/* Compare two strings */
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
    vga_print_color("  help   ", COLOR_GREEN, COLOR_BLACK);
    vga_print("- Show this help message\n");
    vga_print_color("  clear  ", COLOR_GREEN, COLOR_BLACK);
    vga_print("- Clear the screen\n");
    vga_print_color("  about  ", COLOR_GREEN, COLOR_BLACK);
    vga_print("- About myOS\n");
    vga_print_color("  echo   ", COLOR_GREEN, COLOR_BLACK);
    vga_print("- Echo text back (e.g. echo hello)\n");
    vga_print_color("  shutdown ", COLOR_GREEN, COLOR_BLACK);
    vga_print("- Power off the system\n");
}

static void cmd_clear() {
    vga_clear();
}

static void cmd_about() {
    vga_print_color("\n  myOS v0.2\n", COLOR_CYAN, COLOR_BLACK);
    vga_print("  A simple x86 OS built from scratch\n");
    vga_print("  Arch: x86 32-bit | Boot: GRUB Multiboot\n");
    vga_print("  Features: VGA, GDT, IDT, IRQ, Keyboard\n");
}

static void cmd_echo(const char* args) {
    vga_print("\n");
    vga_print(args);
    vga_print("\n");
}

static void cmd_shutdown() {
    vga_print_color("\nShutting down myOS...\n", COLOR_RED, COLOR_BLACK);
    /* QEMU/ACPI shutdown via port 0x604 */
    __asm__ volatile("outw %0, %1" : : "a"((unsigned short)0x2000), "Nd"((unsigned short)0x604));
}

static void shell_execute() {
    /* Null terminate */
    input_buffer[input_len] = '\0';

    /* Skip empty input */
    if (input_len == 0) {
        shell_prompt();
        return;
    }

    /* Find where args start (after first space) */
    int cmd_end = 0;
    while (cmd_end < input_len && input_buffer[cmd_end] != ' ') {
        cmd_end++;
    }

    /* Temporarily null-terminate the command part */
    char saved = input_buffer[cmd_end];
    input_buffer[cmd_end] = '\0';
    const char* cmd = input_buffer;
    const char* args = (saved == ' ') ? &input_buffer[cmd_end + 1] : "";

    if (str_equals(cmd, "help")) {
        cmd_help();
    } else if (str_equals(cmd, "clear")) {
        cmd_clear();
    } else if (str_equals(cmd, "about")) {
        cmd_about();
    } else if (str_equals(cmd, "echo")) {
        cmd_echo(args);
    } else if (str_equals(cmd, "shutdown")) {
        cmd_shutdown();
    } else {
        vga_print_color("\nUnknown command: ", COLOR_RED, COLOR_BLACK);
        vga_print(cmd);
        vga_print("\nType 'help' for available commands.\n");
    }

    /* Restore buffer and reset */
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