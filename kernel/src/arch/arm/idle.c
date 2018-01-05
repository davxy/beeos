#include "proc.h"
#include "kprintf.h"

void idle()
{
    while (1)
    {
        current_task->state = TASK_SLEEPING;
        scheduler();
    }
}
