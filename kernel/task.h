#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "isr.h"

#define TASK_STACK_SIZE 4096
#define MAX_TASKS       8

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_DEAD
} task_state_t;

struct task {
    uint32_t esp;
    uint32_t* stack;
    task_state_t state;
    char name[32];
    uint32_t id;
};

void tasking_init();
void task_create(const char* name, void (*entry)());
void task_exit();
void schedule(struct registers* r);
int tasking_ready();
void task_list();

#endif