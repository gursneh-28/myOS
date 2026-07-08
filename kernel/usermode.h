#ifndef USERMODE_H
#define USERMODE_H

#include <stdint.h>

void enter_usermode(void (*func)());

/* Called from asm */
void enter_usermode_asm(uint32_t user_cs, uint32_t user_ds,
                        uint32_t user_esp, void (*func)());

#endif