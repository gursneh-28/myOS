#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include "isr.h"

/* Syscall numbers */
#define SYS_PRINT   0
#define SYS_EXIT    1
#define SYS_GETPID  2
#define SYS_UPTIME  3

void syscall_init();
void syscall_dispatch(struct registers* r);

#endif