#include "usermode.h"
#include "gdt.h"
#include "../drivers/vga.h"

#define USER_CODE_SEG  0x1B   /* GDT slot 3 | ring 3 */
#define USER_DATA_SEG  0x23   /* GDT slot 4 | ring 3 */

static uint8_t user_stack[4096] __attribute__((aligned(16)));

void enter_usermode(void (*func)()) {
    vga_print_color("[OK] ", COLOR_GREEN, COLOR_BLACK);
    vga_print("Entering user mode (Ring 3)...\n");

    uint32_t kernel_esp;
    __asm__ volatile("mov %%esp, %0" : "=r"(kernel_esp));
    tss_set_kernel_stack(kernel_esp);

    uint32_t user_esp = (uint32_t)(user_stack + sizeof(user_stack));

    enter_usermode_asm(USER_CODE_SEG, USER_DATA_SEG, user_esp, func);
}