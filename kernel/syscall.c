#include "syscall.h"
#include "isr.h"
#include "idt.h"
#include "task.h"
#include "../drivers/vga.h"
#include "../drivers/timer.h"

static void print_int(uint32_t val) {
    if (val == 0) { vga_print("0"); return; }
    char buf[12]; int i = 11; buf[i] = '\0';
    while (val > 0) { buf[--i] = '0' + (val % 10); val /= 10; }
    vga_print(&buf[i]);
}

void syscall_dispatch(struct registers* r) {
    uint32_t syscall_num = r->eax;
    uint32_t arg1        = r->ebx;

    switch (syscall_num) {
        case SYS_PRINT: {
            const char* str = (const char*)arg1;
            if (str) vga_print(str);
            r->eax = 0;
            break;
        }
        case SYS_EXIT: {
            task_exit();
            break;
        }
        case SYS_GETPID: {
            r->eax = 0;
            break;
        }
        case SYS_UPTIME: {
            r->eax = timer_get_seconds();
            break;
        }
        default: {
            vga_print_color("\n[SYSCALL] Unknown: ", COLOR_RED, COLOR_BLACK);
            print_int(syscall_num);
            vga_print("\n");
            r->eax = (uint32_t)-1;
            break;
        }
    }
}

void syscall_init() {
    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("Syscalls initialized (int 0x80)\n");
}