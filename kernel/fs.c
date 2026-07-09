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

    fs_create_demo_elf();

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

void fs_create_demo_elf() {
    static uint8_t elf_binary[] = {
        /* ELF Header (52 bytes) */
        0x7F,'E','L','F',
        0x01, 0x01, 0x01, 0x00,
        0,0,0,0,0,0,0,0,
        0x02,0x00,
        0x03,0x00,
        0x01,0x00,0x00,0x00,
        0x54,0x00,0x20,0x00,    /* entry = 0x200054 */
        0x34,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x34,0x00,
        0x20,0x00,
        0x01,0x00,
        0x00,0x00,
        0x00,0x00,
        0x00,0x00,

        /* Program Header (32 bytes) */
        0x01,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x20,0x00,    /* vaddr = 0x200000 */
        0x00,0x00,0x20,0x00,
        0x90,0x00,0x00,0x00,    /* file_size = 144 */
        0x90,0x00,0x00,0x00,    /* mem_size  = 144 */
        0x05,0x00,0x00,0x00,
        0x00,0x10,0x00,0x00,

        /* Code at 0x200054:
           mov eax, 0          ; SYS_PRINT
           mov ebx, 0x200068   ; message address
           int 0x80            ; syscall
           ret                 ; return to elf_load() */
        0xB8,0x00,0x00,0x00,0x00,   /* mov eax, 0  */
        0xBB,0x68,0x00,0x20,0x00,   /* mov ebx, 0x200068 */
        0xCD,0x80,                   /* int 0x80 */
        0xC3,                        /* ret  ← just return! */

        /* Message at 0x200068 */
        '[','E','L','F',']',' ',
        'H','e','l','l','o',' ',
        'f','r','o','m',' ',
        'a',' ','u','s','e','r',' ',
        'p','r','o','g','r','a','m','!',
        '\n','\0',

        /* Padding to 144 bytes */
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0
    };

    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) {
            files[i].name[0]='h'; files[i].name[1]='e';
            files[i].name[2]='l'; files[i].name[3]='l';
            files[i].name[4]='o'; files[i].name[5]='.';
            files[i].name[6]='e'; files[i].name[7]='l';
            files[i].name[8]='f'; files[i].name[9]='\0';

            uint32_t sz = sizeof(elf_binary);
            if (sz > MAX_FILESIZE) sz = MAX_FILESIZE;
            for (uint32_t j = 0; j < sz; j++)
                files[i].data[j] = elf_binary[j];
            files[i].size = sz;
            files[i].used = 1;
            file_count++;
            break;
        }
    }
}