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

#include "cond.h"
#include "proc.h"


void cond_init(struct cond *cv)
{
    spinlock_init(&cv->lock);
    list_init(&cv->queue);
}

void cond_wait(struct cond *cv)
{
    list_insert_before(&cv->queue, &current->condw);
    current->state = TASK_SLEEPING;

    spinlock_unlock(&cv->lock);
    scheduler();
    spinlock_lock(&cv->lock);
}

void cond_signal(struct cond *cv)
{
    struct task *t;

    if (list_empty(&cv->queue))
        return;
    t = struct_ptr(cv->queue.next, struct task, condw);
    list_delete(&t->condw);
    t->state = TASK_RUNNING;
}

void cond_broadcast(struct cond *cv)
{
    while (!list_empty(&cv->queue))
        cond_signal(cv);
}
