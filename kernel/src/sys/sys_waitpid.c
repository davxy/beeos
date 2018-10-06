/*
 * Copyright (c) 2015-2018, Davide Galassi. All rights reserved.
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

    spinlock_lock(&current->chld_exit.lock);

    do {
        retry = 0;
        havekids = 0;

        t = struct_ptr(current->tasks.next, struct task, tasks);
        while (t != current) {
            if (t->pptr == current && (pid == t->pid || pid == -1)) {
                havekids = 1;
                if (t->state == TASK_ZOMBIE) {
                    /* found one */
                    pid = t->pid;
                    if (wstatus != NULL)
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

        if (t == current) {
            /* We've not found any terminated children */
            if (havekids != 0) {
                /* There are not terminated children around */
                if ((options & WNOHANG) == 0) {
                    /* WNOHANG flag not specified, wait for a child */
                    cond_wait(&current->chld_exit);
                    retry = 1;
                } else {
                    pid = 0;
                }
            } else {
                pid = -1;
            }
        }
    } while (retry != 0);

    spinlock_unlock(&current->chld_exit.lock);

    return pid;
}
