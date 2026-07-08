#include "task.h"
#include "heap.h"
#include "isr.h"
#include "../drivers/vga.h"

static struct task tasks[MAX_TASKS];
static int current_task = -1;
static int task_count   = 0;
static int initialized  = 0;

static void print_int(uint32_t val) {
    if (val == 0) { vga_print("0"); return; }
    char buf[12]; int i = 11; buf[i] = '\0';
    while (val > 0) { buf[--i] = '0' + (val % 10); val /= 10; }
    vga_print(&buf[i]);
}

void schedule(struct registers* r) {
    if (!initialized || task_count < 2) return;

    /* Save current task's stack pointer */
    if (current_task >= 0) {
        tasks[current_task].esp = (uint32_t)r;
    }

    /* Find next READY task (round robin) */
    int next = current_task;
    for (int i = 0; i < MAX_TASKS; i++) {
        next = (next + 1) % MAX_TASKS;
        if (tasks[next].state == TASK_READY || tasks[next].state == TASK_RUNNING) {
            break;
        }
    }

    if (next == current_task) return;

    if (current_task >= 0) tasks[current_task].state = TASK_READY;
    tasks[next].state = TASK_RUNNING;
    current_task = next;

    /* Switch to next task's stack and restore its context */
    __asm__ volatile(
        "movl %0, %%esp\n"       /* restore esp */
        "popl %%eax\n"           /* pop saved ds */
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        "popa\n"
        "addl $8, %%esp\n"       /* skip int_no and err_code */
        "iret\n"
        : : "r"(tasks[current_task].esp)
    );
}

void task_exit() {
    if (current_task >= 0) {
        tasks[current_task].state = TASK_DEAD;
        task_count--;
        current_task = -1;
    }
    /* Return to caller — kernel_main will continue to shell_init */
}

void task_create(const char* name, void (*entry)()) {
    if (task_count >= MAX_TASKS) return;

    /* Find free slot */
    int slot = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_DEAD || tasks[i].stack == 0) {
            slot = i; break;
        }
    }
    if (slot == -1) return;

    /* Allocate stack */
    uint8_t* stack = (uint8_t*)kmalloc(TASK_STACK_SIZE);
    if (!stack) return;

    /* Copy name */
    int j = 0;
    while (name[j] && j < 31) { tasks[slot].name[j] = name[j]; j++; }
    tasks[slot].name[j] = '\0';

    tasks[slot].id    = slot;
    tasks[slot].stack = (uint32_t*)stack;
    tasks[slot].state = TASK_READY;

    /* Set up initial stack frame to look like an interrupted task
       Stack grows downward, set up as if IRQ fired while task was running */
    uint32_t* stk = (uint32_t*)(stack + TASK_STACK_SIZE);

    /* iret frame */
    *--stk = 0x10;          /* ss */
    *--stk = (uint32_t)(stack + TASK_STACK_SIZE - 64); /* useresp */
    *--stk = 0x202;         /* eflags: interrupts enabled */
    *--stk = 0x08;          /* cs */
    *--stk = (uint32_t)entry; /* eip */

    /* err_code and int_no */
    *--stk = 0;
    *--stk = 32;

    /* ds */
    *--stk = 0x10;

    /* pusha: edi esi ebp esp ebx edx ecx eax */
    *--stk = 0; /* edi */
    *--stk = 0; /* esi */
    *--stk = 0; /* ebp */
    *--stk = 0; /* esp (ignored by popa) */
    *--stk = 0; /* ebx */
    *--stk = 0; /* edx */
    *--stk = 0; /* ecx */
    *--stk = 0; /* eax */

    tasks[slot].esp = (uint32_t)stk;
    task_count++;

    vga_print_color("[TASK] ", COLOR_BROWN, COLOR_BLACK);
    vga_print("Created task '");
    vga_print(tasks[slot].name);
    vga_print("' (id=");
    print_int(slot);
    vga_print(")\n");
}

void tasking_init() {
    /* Mark all slots empty */
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].state = TASK_DEAD;
        tasks[i].stack = 0;
        tasks[i].id    = i;
    }

    /* Register scheduler on timer IRQ0 */
    irq_install_handler(0, schedule);

    initialized = 1;

    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("Tasking initialized\n");
}

int tasking_ready() { return initialized; }

void task_list() {
    vga_print_color("\n--- Task List ---\n", COLOR_CYAN, COLOR_BLACK);
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].stack == 0) continue;
        vga_print("  [");
        print_int(tasks[i].id);
        vga_print("] ");
        vga_print(tasks[i].name);
        vga_print(" - ");
        if (tasks[i].state == TASK_RUNNING)
            vga_print_color("RUNNING\n", COLOR_GREEN, COLOR_BLACK);
        else if (tasks[i].state == TASK_READY)
            vga_print_color("READY\n",   COLOR_CYAN,  COLOR_BLACK);
        else
            vga_print_color("DEAD\n",    COLOR_RED,   COLOR_BLACK);
    }
    vga_print_color("-----------------\n", COLOR_CYAN, COLOR_BLACK);
}