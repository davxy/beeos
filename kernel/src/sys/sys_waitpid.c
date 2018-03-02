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
#include "util.h"
#include "kprintf.h"    // TODO : remove
#include "kmalloc.h"
#include <sys/wait.h>

/*
 * Wait for a child process to exit and return its pid.
 * Return -1 if this process has no children.
 */
pid_t sys_waitpid(pid_t pid, int *wstatus, int options)
{
    struct task *t;
    int havekids;
    int retry;

    spinlock_lock(&current_task->chld_exit.lock);

    do {
        retry = 0;
        havekids = 0;

        t = struct_ptr(current_task->tasks.next, struct task, tasks);
        while (t != current_task)
        {
            if (t->pptr == current_task
                && (pid == t->pid || pid == -1))
            {
                havekids = 1;
                if (t->state == TASK_ZOMBIE)
                {
                    /* found one */
                    pid = t->pid;
                    if (wstatus)
                        *wstatus = t->exit_code;
                    /* resources already released by the sys_exit */
                    list_delete(&t->tasks);
                    list_delete(&t->children);
                    list_delete(&t->sibling);
                    task_delete(t);
                    break;
                }
            }
            t = struct_ptr(t->tasks.next, struct task, tasks);
        }

        if (t == current_task)
        {
            /* We've not found any terminated children */
            if (havekids)
            {
                /* There are not terminated children around */
                if ((options & WNOHANG) == 0)
                {
                    /* WNOHANG flag not specified, wait for a child */
                    cond_wait(&current_task->chld_exit);
                    retry = 1;
                }
                else
                {
                    pid = 0;
                }
            }
            else
            {
                pid = -1;
            }
        }
    } while (retry != 0);

    spinlock_unlock(&current_task->chld_exit.lock);

    return pid;
}
