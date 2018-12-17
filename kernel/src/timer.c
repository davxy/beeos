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

#include "timer.h"
#include "proc.h"

clock_t timer_ticks = 0;

/* Timer events queue. */
static struct list_link timer_events;

/**
 * Architecture dependent timer initialization.
 */
void timer_arch_init(void);


void timer_event_add(struct timer_event *tm)
{
    const struct timer_event *e;
    struct list_link *curr;

    curr = timer_events.next;
    while (curr != &timer_events) {
        e = list_container(curr, struct timer_event, link);
        if (e->expires > tm->expires)
            break;
        curr = curr->next;
    }
    list_insert_before(curr, &tm->link);
}

void timer_event_del(struct timer_event *tm)
{
    list_delete(&tm->link);
}

void timer_event_mod(struct timer_event *tm, unsigned long expires)
{
    tm->expires = expires;
    timer_event_add(tm);
}

void timer_event_init(struct timer_event *tm, timer_event_t *fn,
        void *data, unsigned long expires)
{
    list_init(&tm->link);
    list_init(&tm->plink);
    tm->func = fn;
    tm->data = data;
    tm->expires = expires;
}

void timer_update(void)
{
    struct timer_event *tm;
    struct list_link *curr = timer_events.next;

    while (curr != &timer_events) {
        tm = list_container(curr, struct timer_event, link);
        curr = curr->next;
        if (tm->expires <= timer_ticks) {
            list_delete(&tm->link);
            tm->func(tm->data);
        }
    }

    if (current->counter-- <= 0)
        need_resched = 1;
}

void timer_init(void)
{
    list_init(&timer_events);
    timer_arch_init();
}
