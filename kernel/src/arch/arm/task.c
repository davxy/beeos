#include "proc/task.h"

int task_arch_init(struct task_arch *task)
{
    return 0;
}

void task_arch_deinit(struct task_arch *task)
{
}

void task_arch_switch(struct task_arch *curr, struct task_arch *next)
{
}
