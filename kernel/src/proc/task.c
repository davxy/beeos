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

#include "task.h"
#include "proc.h"
#include "fs/vfs.h"
#include "timer.h"
#include "kmalloc.h"
#include "panic.h"
#include <string.h>


void task_signal(struct task *tsk, int sig)
{
    sigaddset(&tsk->sigpend, sig);

    /* check if signal is not masked */
    if (sigismember(&tsk->sigmask, sig) <= 0) {
        /* check if the process must be awake */
        if (tsk->state == TASK_SLEEPING) {
            if (!list_empty(&tsk->condw))
                list_delete(&tsk->condw);
            tsk->state = TASK_RUNNING;
        }
    }
}

int task_init(struct task *tsk, task_entry_t entry)
{
    static pid_t next_pid = 1;
    int i;
    struct task *sib;

    /* pids */
    tsk->pid = next_pid++;
    tsk->pgid = current->pgid;
    tsk->pptr = current;

    /* user and group */
    tsk->uid = current->uid;
    tsk->euid = current->euid;
    tsk->suid = current->suid;
    tsk->gid = current->gid;
    tsk->egid = current->egid;
    tsk->sgid = current->sgid;

    /* file system */
    tsk->cwd = ddup(current->cwd);
    tsk->root = ddup(current->root);

    /* duplicate valid file descriptors */
    memset(tsk->fds, 0, sizeof(tsk->fds));
    for (i = 0; i < OPEN_MAX; i++) {
        if (current->fds[i].fil != NULL) {
            tsk->fds[i] = current->fds[i];
            tsk->fds[i].fil->ref++;
        }
    }

    /* memory */
    tsk->brk = current->brk;

    /* sheduler */
    tsk->usage = 0;
    tsk->state = TASK_RUNNING;
    tsk->counter = msecs_to_ticks(SCHED_TIMESLICE);
    tsk->exit_code = 0;

    list_init(&tsk->tasks);
    list_init(&tsk->children);
    list_init(&tsk->sibling);

    /* Add to the global tasks list */
    list_insert_before(&current->tasks, &tsk->tasks);

    sib = list_container(current->children.next, struct task, children);
    if (list_empty(&current->children) || sib->pptr != current)
        list_insert_after(&current->children, &tsk->children);
    else
        list_insert_before(&sib->sibling, &tsk->sibling);

    cond_init(&tsk->chld_exit);

    /* signals */
    sigemptyset(&tsk->sigpend);
    sigemptyset(&tsk->sigmask);
    memcpy(tsk->signals, current->signals, sizeof(tsk->signals));

    /* Timers events */
    list_init(&tsk->timers);

    /* Conditional wait link */
    list_init(&tsk->condw);

    /* Controlling terminal */
    tsk->tty = current->tty;

    return task_arch_init(&tsk->arch, entry);
}


void task_deinit(struct task *tsk)
{
    dput(tsk->cwd);
    dput(tsk->root);
    task_arch_deinit(&tsk->arch);
}


struct task *task_create(task_entry_t entry)
{
    struct task *tsk;

    tsk = (struct task *)kmalloc(sizeof(struct task), 0);
    if (tsk != NULL) {
        memset(tsk, 0, sizeof(*tsk));
        if (task_init(tsk, entry) < 0) {
            kfree(tsk, sizeof(struct task));
            tsk = NULL;
        }
    }
    return tsk;
}


void task_delete(struct task *tsk)
{
    task_deinit(tsk);
    kfree(tsk, sizeof(struct task));
}
