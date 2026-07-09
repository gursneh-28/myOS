#include "shell.h"
#include "../drivers/vga.h"
#include "../kernel/heap.h"
#include "../kernel/pmm.h"
#include "../drivers/timer.h"
#include "../kernel/paging.h"
#include "../kernel/task.h"
#include "../kernel/fs.h"
#include "../kernel/elf.h"

/* Command history */
#define HISTORY_SIZE 8
#define INPUT_BUFFER_SIZE 256

static char input_buffer[INPUT_BUFFER_SIZE];
static int  input_len = 0;

static char history[HISTORY_SIZE][INPUT_BUFFER_SIZE];
static int  history_count = 0;   /* total commands stored */
static int  history_idx   = -1;  /* current position when browsing (-1 = not browsing) */

static void print_int(uint32_t val) {
    if (val == 0) { vga_print("0"); return; }
    char buf[12]; int i = 11; buf[i] = '\0';
    while (val > 0) { buf[--i] = '0' + (val % 10); val /= 10; }
    vga_print(&buf[i]);
}

static void shell_prompt() {
    vga_print_color("\n> ", COLOR_GREEN, COLOR_BLACK);
}

static int str_equals(const char* a, const char* b) {
    int i = 0;
    while (a[i] && b[i]) { if (a[i] != b[i]) return 0; i++; }
    return a[i] == '\0' && b[i] == '\0';
}

static void cmd_help() {
    vga_print_color("\nAvailable commands:\n", COLOR_CYAN, COLOR_BLACK);
    vga_print_color("  help     ", COLOR_GREEN, COLOR_BLACK); vga_print("- Show this help message\n");
    vga_print_color("  clear    ", COLOR_GREEN, COLOR_BLACK); vga_print("- Clear the screen\n");
    vga_print_color("  about    ", COLOR_GREEN, COLOR_BLACK); vga_print("- About myOS\n");
    vga_print_color("  echo     ", COLOR_GREEN, COLOR_BLACK); vga_print("- Echo text (e.g. echo hello)\n");
    vga_print_color("  meminfo  ", COLOR_GREEN, COLOR_BLACK); vga_print("- Show memory usage\n");
    vga_print_color("  paging   ", COLOR_GREEN, COLOR_BLACK); vga_print("- Show paging info\n");
    vga_print_color("  uptime   ", COLOR_GREEN, COLOR_BLACK); vga_print("- Show time since boot\n");
    vga_print_color("  version  ", COLOR_GREEN, COLOR_BLACK); vga_print("- Show OS version info\n");
    vga_print_color("  reboot   ", COLOR_GREEN, COLOR_BLACK); vga_print("- Reboot the system\n");
    vga_print_color("  shutdown ", COLOR_GREEN, COLOR_BLACK); vga_print("- Power off the system\n");
    vga_print_color("  tasks    ", COLOR_GREEN, COLOR_BLACK); vga_print("- List running tasks\n");
    vga_print_color("  ls       ", COLOR_GREEN, COLOR_BLACK); vga_print("- List files\n");
    vga_print_color("  cat      ", COLOR_GREEN, COLOR_BLACK); vga_print("- Print file (e.g. cat readme.txt)\n");
    vga_print_color("  write    ", COLOR_GREEN, COLOR_BLACK); vga_print("- Create file (e.g. write name.txt content)\n");
    vga_print_color("  rm       ", COLOR_GREEN, COLOR_BLACK); vga_print("- Delete file (e.g. rm name.txt)\n");
    vga_print_color("  run      ", COLOR_GREEN, COLOR_BLACK); vga_print("- Run ELF program (e.g. run hello.elf)\n");
}

static void cmd_clear() { vga_clear(); }

static void cmd_about() {
    vga_print_color("\n  myOS v0.2\n", COLOR_CYAN, COLOR_BLACK);
    vga_print("  A simple x86 OS built from scratch\n");
    vga_print("  Arch    : x86 32-bit\n");
    vga_print("  Boot    : GRUB Multiboot\n");
    vga_print("  Features: VGA, GDT, IDT, PIC, IRQ,\n");
    vga_print("            Keyboard, PMM, Heap, Paging,\n");
    vga_print("            Scheduler, Syscalls, Ring3, FS\n");
}

static void cmd_echo(const char* args) {
    vga_print("\n"); vga_print(args); vga_print("\n");
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

static void cmd_paging() {
    vga_print_color("\n--- Paging Info ---\n", COLOR_CYAN, COLOR_BLACK);
    vga_print("  Status : enabled\n");
    vga_print("  Mode   : 32-bit, 4KB pages\n");
    vga_print("  Mapped : 0x00000000 - 0x003FFFFF (4MB identity)\n");
}

static void cmd_uptime() {
    uint32_t seconds = timer_get_seconds();
    uint32_t minutes = seconds / 60;
    uint32_t hours   = minutes / 60;
    seconds %= 60; minutes %= 60;
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

static void cmd_tasks() { task_list(); }

static void cmd_ls() { fs_list(); }

static void cmd_cat(const char* filename) {
    if (!filename || filename[0] == '\0') {
        vga_print_color("\nUsage: cat <filename>\n", COLOR_RED, COLOR_BLACK);
        return;
    }
    struct file* f = fs_open(filename);
    if (!f) {
        vga_print_color("\nFile not found: ", COLOR_RED, COLOR_BLACK);
        vga_print(filename);
        vga_print("\n");
        return;
    }
    vga_print("\n");
    vga_print(f->data);
}

static void cmd_write(const char* args) {
    /* args = "filename content" */
    if (!args || args[0] == '\0') {
        vga_print_color("\nUsage: write <filename> <content>\n", COLOR_RED, COLOR_BLACK);
        return;
    }

    /* Split filename and content */
    int i = 0;
    while (args[i] && args[i] != ' ') i++;

    if (args[i] == '\0') {
        vga_print_color("\nUsage: write <filename> <content>\n", COLOR_RED, COLOR_BLACK);
        return;
    }

    /* Copy filename */
    char fname[32];
    int j = 0;
    while (j < i && j < 31) { fname[j] = args[j]; j++; }
    fname[j] = '\0';

    const char* content = &args[i + 1];

    if (fs_create(fname, content) == 0) {
        vga_print_color("\nFile created: ", COLOR_GREEN, COLOR_BLACK);
        vga_print(fname);
        vga_print("\n");
    } else {
        vga_print_color("\nFailed to create file (duplicate or full)\n", COLOR_RED, COLOR_BLACK);
    }
}

static void cmd_rm(const char* filename) {
    if (!filename || filename[0] == '\0') {
        vga_print_color("\nUsage: rm <filename>\n", COLOR_RED, COLOR_BLACK);
        return;
    }
    if (fs_delete(filename) == 0) {
        vga_print_color("\nDeleted: ", COLOR_GREEN, COLOR_BLACK);
        vga_print(filename);
        vga_print("\n");
    } else {
        vga_print_color("\nFile not found: ", COLOR_RED, COLOR_BLACK);
        vga_print(filename);
        vga_print("\n");
    }
}

static void cmd_run(const char* filename) {
    if (!filename || filename[0] == '\0') {
        vga_print_color("\nUsage: run <filename>\n", COLOR_RED, COLOR_BLACK);
        return;
    }
    elf_load(filename);
}

static void history_push(const char* cmd) {
    if (cmd[0] == '\0') return;
    /* Shift history down */
    for (int i = HISTORY_SIZE - 1; i > 0; i--) {
        for (int j = 0; j < INPUT_BUFFER_SIZE; j++) {
            history[i][j] = history[i-1][j];
        }
    }
    /* Copy new command to slot 0 */
    int i = 0;
    while (cmd[i] && i < INPUT_BUFFER_SIZE - 1) {
        history[0][i] = cmd[i]; i++;
    }
    history[0][i] = '\0';
    if (history_count < HISTORY_SIZE) history_count++;
    history_idx = -1;
}

static void shell_clear_line() {
    /* Erase current input from screen */
    while (input_len > 0) {
        vga_backspace();
        input_len--;
    }
}

static void shell_set_input(const char* str) {
    shell_clear_line();
    int i = 0;
    while (str[i] && i < INPUT_BUFFER_SIZE - 1) {
        input_buffer[i] = str[i];
        char s[2] = {str[i], '\0'};
        vga_print_color(s, COLOR_GREEN, COLOR_BLACK);
        i++;
    }
    input_buffer[i] = '\0';
    input_len = i;
}

void shell_history_up() {
    if (history_count == 0) return;
    if (history_idx < history_count - 1) history_idx++;
    shell_set_input(history[history_idx]);
}

void shell_history_down() {
    if (history_idx <= 0) {
        history_idx = -1;
        shell_clear_line();
        return;
    }
    history_idx--;
    shell_set_input(history[history_idx]);
}

static void shell_execute() {
    input_buffer[input_len] = '\0';
    history_push(input_buffer);   /* ← add this line */
    history_idx = -1;             /* ← and this */

    if (input_len == 0) { shell_prompt(); return; }

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
    else if (str_equals(cmd, "paging"))   cmd_paging();
    else if (str_equals(cmd, "uptime"))   cmd_uptime();
    else if (str_equals(cmd, "version"))  cmd_version();
    else if (str_equals(cmd, "reboot"))   cmd_reboot();
    else if (str_equals(cmd, "shutdown")) cmd_shutdown();
    else if (str_equals(cmd, "tasks"))    cmd_tasks();
    else if (str_equals(cmd, "ls"))       cmd_ls();
    else if (str_equals(cmd, "cat"))      cmd_cat(args);
    else if (str_equals(cmd, "write"))    cmd_write(args);
    else if (str_equals(cmd, "rm"))       cmd_rm(args);
    else if (str_equals(cmd, "run")) cmd_run(args);
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
        if (input_len > 0) { input_len--; vga_backspace(); }
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