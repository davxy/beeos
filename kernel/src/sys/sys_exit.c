/*
 * Copyright (c) 2015-2017, Davide Galassi. All rights reserved.
 *
 * This file is part of the BeeOS software.
 *
 * BeeOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BeeOS; if not, see <http://www.gnu/licenses/>.
 */

#include "sys.h"
#include "proc.h"
#include "kprintf.h"
#include "panic.h"
#include <sys/types.h>
#include <stddef.h>

/*
 * An exited process remains in the zombie state until its parent
 * calls wait() to find out it exited.
 */
void sys_exit(int status)
{
    struct task *t, *init = NULL;
    struct list_link *lnk;
    struct timer_event *tm;
    int i;

    //kprintf("[sys_exit] %d exiting\n", current_task->pid);

    if (current_task->pid == 1)
        panic("init exiting");

    /* Flush the timer event queue */
    while (!list_empty(&current_task->timers))
    {
        lnk = current_task->timers.next;
        list_delete(lnk);    /* Remove from current process timers */
        tm = list_container(lnk, struct timer_event, plink); 
        timer_event_del(tm); /* Remove from the global queue */
        // TODO: fix memory leak
        // Almost every timer is a dynamic structure. How to determine
        // it??? Use a flag within a timer???
    }

    /* close all open files */
    for (i = 0; i < OPEN_MAX; i++)
    {
        if (current_task->fd[i].file)
            sys_close(i);
    }

    iput(current_task->cwd);
    current_task->cwd = NULL;
   
    /* Pass abandoned children to init */

    t = list_container(current_task->tasks.next, 
            struct task, tasks);
    while (t != current_task)
    {
        if (t->pid == 1)
        {
            init = t;
            break;
        }
        t = list_container(t->tasks.next, struct task, tasks);
    }
    if (init == NULL)
        panic("init process not found");

    t = list_container(current_task->tasks.next, 
            struct task, tasks);
    while (t != current_task)
    {
        if (t->pptr == current_task)
            t->pptr = init;
        t = list_container(t->tasks.next, struct task, tasks);
    }


    /* Send SIGCHLD to the parent */
    sys_kill(current_task->pptr->pid, SIGCHLD);

    /* Acquire the father conditional variable to be prevent lost signals */
    spinlock_lock(&current_task->pptr->chld_exit.lock);
    current_task->state = TASK_ZOMBIE;
    current_task->exit_code = status;
    cond_signal(&current_task->pptr->chld_exit);
    spinlock_unlock(&current_task->pptr->chld_exit.lock);

    scheduler();
    panic("zombie process resumed\n");
}
