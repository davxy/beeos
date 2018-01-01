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

static struct task *find_init(void)
{
    struct task *t;
    extern struct task ktask;

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
    struct task *head, *curr, *prev;

    head = NULL;
    prev = node;
    curr = list_container(prev->children.next, struct task, children);
    while (curr != node) {
        if (curr->pptr != prev) {
            head = curr;
            break;
        } prev = curr;
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
    struct task *init, *init_child, *t;

    init = find_init();
    init_child = list_container(init->children.next,
                                struct task, children);
    if (init_child->pid == 0)
        panic("init has not childs");

    /* node is in the middle of the children chain */
    children_split(current_task);
    t = child;
    do {
        if (t->pptr != current_task)
            panic("corrupted sibling list");
        t->pptr = init; /* give in adoption */
        t = list_container(t->sibling.next, struct task, sibling);
    } while (t != child);

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
   
    /* Give children to init */
    child = list_container(current_task->children.next,
                           struct task, children);
    if (child->pptr == current_task)
        children_give(child); /* current is not a hierarchy leaf */

    /* Send SIGCHLD to the parent */
    sys_kill(current_task->pptr->pid, SIGCHLD);

    /* Acquire the father conditional variable to prevent lost signals */
    spinlock_lock(&current_task->pptr->chld_exit.lock);
    current_task->state = TASK_ZOMBIE;
    current_task->exit_code = status;
    cond_signal(&current_task->pptr->chld_exit);
    spinlock_unlock(&current_task->pptr->chld_exit.lock);

    scheduler();
    panic("zombie process resumed\n");
}
