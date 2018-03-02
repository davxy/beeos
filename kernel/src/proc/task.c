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

#include "task.h"
#include "proc.h"
#include "fs/vfs.h"
#include "timer.h"
#include "kmalloc.h"
#include "panic.h"
#include <string.h>


int task_init(struct task *task, task_entry_t entry)
{
    static pid_t next_pid = 1;
    int i;
    struct task *sib;

    /* pids */
    task->pid = next_pid++;
    task->pgid = current_task->pgid;
    task->pptr = current_task;

    /* user and group */
    task->uid = current_task->uid;
    task->euid = current_task->euid;
    task->suid = current_task->suid;
    task->gid = current_task->gid;
    task->egid = current_task->egid;
    task->sgid = current_task->sgid;

    /* file system */
    task->cwd = current_task->cwd;
    task->root = current_task->root;
    dget(task->cwd);
    dget(task->root);

    /* duplicate valid file descriptors */
    memset(task->fds, 0, sizeof(task->fds));
    for (i = 0; i < OPEN_MAX; i++)
    {
        if (!current_task->fds[i].fil)
            continue;
        task->fds[i] = current_task->fds[i];
        task->fds[i].fil->ref++;
    }

    /* memory */
    task->brk = current_task->brk;

    /* sheduler */
    task->state = TASK_RUNNING;
    task->counter = msecs_to_ticks(SCHED_TIMESLICE);
    task->exit_code = 0;

    list_init(&task->tasks);
    list_init(&task->children);
    list_init(&task->sibling);

    /* Add to the global tasks list */
    list_insert_before(&current_task->tasks, &task->tasks);

    sib = list_container(current_task->children.next, struct task, children);
    if (list_empty(&current_task->children) || sib->pptr != current_task)
        list_insert_after(&current_task->children, &task->children);
    else
        list_insert_before(&sib->sibling, &task->sibling);

    cond_init(&task->chld_exit);

    /* signals */
    (void)sigemptyset(&task->sigpend);
    (void)sigemptyset(&task->sigmask);
    memcpy(task->signals, current_task->signals, sizeof(task->signals));

    /* Timers events */
    list_init(&task->timers);

    /* Conditional wait link */
    list_init(&task->condw);

    task_arch_init(&task->arch, entry);

    return 0;
}


void task_deinit(struct task *task)
{
    dput(task->cwd);
    dput(task->root);
    task_arch_deinit(&task->arch);
}


struct task *task_create(task_entry_t entry)
{
    struct task *task = kmalloc(sizeof(struct task), 0);
    if (task)
    {
        memset(task, 0, sizeof(*task));
        task_init(task, entry);
    }
    return task;
}


void task_delete(struct task *task)
{
    task_deinit(task);
    kfree(task, sizeof(struct task));
}

