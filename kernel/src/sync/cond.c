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

#include "cond.h"
#include "proc.h"
#include "kmalloc.h"


void cond_init(struct cond *cond)
{
    spinlock_init(&cond->lock);
    list_init(&cond->queue);
}

void cond_wait(struct cond *cond)
{
    list_insert_before(&cond->queue, &current_task->condw);
    current_task->state = TASK_SLEEPING;

    spinlock_unlock(&cond->lock);
    scheduler();
    spinlock_lock(&cond->lock);
}

void cond_signal(struct cond *cond)
{
    struct task *t;

    if (list_empty(&cond->queue))
        return;
    t = struct_ptr(cond->queue.next, struct task, condw);
    list_delete(&t->condw);
    t->state = TASK_RUNNING;
}

void cond_broadcast(struct cond *cond)
{
    while (!list_empty(&cond->queue))
        cond_signal(cond);
}

