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
#include "kprintf.h"
#include "panic.h"
#include <sys/types.h>
#include <stddef.h>

static struct task *find_init(void)
{
    struct task *t;

    t = list_container(ktask.tasks.next, struct task, tasks);
    while (t != &ktask) {
        if (t->pid == 1)
            break;
        t = list_container(t->tasks.next, struct task, tasks);
    }
    if (t == &ktask)
        panic("init process not found");
    return t;
}

static void children_split(struct task *node)
{
    struct task *head, *curr;
    const struct task *prev;

    head = NULL;
    prev = node;
    curr = list_container(prev->children.next, struct task, children);
    while (curr != node) {
        if (curr->pptr != prev) {
            head = curr;
            break;
        }
        prev = curr;
        curr = list_container(curr->children.next, struct task, children);
    }
    if (head == NULL)
        panic("corrupted children hierarchy");

    /* second half */
    node->children.next->prev = head->children.prev;
    head->children.prev->next = node->children.next;

    /* first hald */
    node->children.next = &head->children;
    head->children.prev = &node->children;
}

static void children_give(struct task *child)
{
    struct task *init_task;
    struct task *init_child;
    struct task *curr_task;

    init_task = find_init();
    init_child = list_container(init_task->children.next,
                                struct task, children);
    if (init_child->pid == 0)
        panic("init has not childs");

    /* node is in the middle of the children chain */
    children_split(current);
    curr_task = child;
    do {
        if (curr_task->pptr != current)
            panic("corrupted sibling list");
        curr_task->pptr = init_task; /* give in adoption */
        /*
         * Wake-up to eventually give the oppurtunity to terminate.
         * This may happen if the process is waiting on a pipe that has
         * been closed on the other side.
         */
        curr_task = list_container(curr_task->sibling.next,
                                   struct task, sibling);
    } while (curr_task != child);

    list_merge(&init_child->sibling, &child->sibling);
}

/*
 * An exited process remains in the zombie state until its parent
 * calls wait() to find out it exited.
 */
void sys_exit(int status)
{
    struct list_link *lnk;
    struct timer_event *tm;
    struct task *child;
    int i;

    if (current->pid == 1)
        panic("init exiting");

    /* Flush the timer event queue */
    while (!list_empty(&current->timers)) {
        lnk = current->timers.next;
        list_delete(lnk);    /* Remove from current process timers */
        tm = list_container(lnk, struct timer_event, plink);
        timer_event_del(tm); /* Remove from the global queue */
    }

    /* close all open files */
    for (i = 0; i < OPEN_MAX; i++) {
        if (current->fds[i].fil != NULL) {
            if (sys_close(i) < 0) {
                /* Should never happen... */
                kprintf("[warn] sys_close error on opened file\n");
            }
        }
    }

    /* Give children to init */
    child = list_container(current->children.next,
                           struct task, children);
    if (child->pptr == current)
        children_give(child); /* Wrap around, current is not a leaf */

    /* Send SIGCHLD to the parent */
    task_signal(current->pptr, SIGCHLD);

    /* Acquire the father conditional variable to prevent lost signals */
    spinlock_lock(&current->pptr->chld_exit.lock);
    current->state = TASK_ZOMBIE;
    current->exit_code = status;
    cond_signal(&current->pptr->chld_exit);
    spinlock_unlock(&current->pptr->chld_exit.lock);

    scheduler();
    panic("zombie process resumed\n");
}
