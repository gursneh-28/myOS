#include "fs.h"
#include "../drivers/vga.h"

static struct file files[MAX_FILES];
static int file_count = 0;

static int str_len(const char* s) {
    int i = 0; while (s[i]) i++; return i;
}

static void str_copy(char* dst, const char* src, int max) {
    int i = 0;
    while (src[i] && i < max - 1) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

static int str_eq(const char* a, const char* b) {
    int i = 0;
    while (a[i] && b[i]) { if (a[i] != b[i]) return 0; i++; }
    return a[i] == '\0' && b[i] == '\0';
}

static void print_int(uint32_t val) {
    if (val == 0) { vga_print("0"); return; }
    char buf[12]; int i = 11; buf[i] = '\0';
    while (val > 0) { buf[--i] = '0' + (val % 10); val /= 10; }
    vga_print(&buf[i]);
}

void fs_init() {
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].used = 0;
    }
    file_count = 0;

    /* Create some default files */
    fs_create("readme.txt",
        "Welcome to myOS!\n"
        "This is a simple x86 OS built from scratch.\n"
        "Type 'help' to see available commands.\n");

    fs_create("about.txt",
        "myOS v0.2\n"
        "Arch: x86 32-bit protected mode\n"
        "Features: VGA, GDT, IDT, IRQ, PMM, Heap,\n"
        "          Paging, Scheduler, Syscalls, Ring3\n"
        "Author: Gursneh\n");

    fs_create("hello.txt", "Hello from the myOS filesystem!\n");

    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("Filesystem initialized | ");
    print_int(file_count);
    vga_print(" files\n");
}

int fs_create(const char* name, const char* data) {
    if (file_count >= MAX_FILES) return -1;

    /* Check duplicate */
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && str_eq(files[i].name, name)) return -1;
    }

    /* Find free slot */
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) {
            str_copy(files[i].name, name, MAX_FILENAME);
            int len = str_len(data);
            if (len >= MAX_FILESIZE) len = MAX_FILESIZE - 1;
            str_copy(files[i].data, data, MAX_FILESIZE);
            files[i].size = len;
            files[i].used = 1;
            file_count++;
            return 0;
        }
    }
    return -1;
}

struct file* fs_open(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && str_eq(files[i].name, name)) {
            return &files[i];
        }
    }
    return 0;
}

int fs_delete(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && str_eq(files[i].name, name)) {
            files[i].used = 0;
            file_count--;
            return 0;
        }
    }
    return -1;
}

void fs_list() {
    vga_print_color("\n--- Files ---\n", COLOR_CYAN, COLOR_BLACK);
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used) {
            vga_print_color("  ", COLOR_GREEN, COLOR_BLACK);
            vga_print(files[i].name);
            vga_print("  (");
            print_int(files[i].size);
            vga_print(" bytes)\n");
            found++;
        }
    }
    if (!found) vga_print("  (no files)\n");
    vga_print_color("-------------\n", COLOR_CYAN, COLOR_BLACK);
    vga_print("  Total: ");
    print_int(found);
    vga_print(" file(s)\n");
}