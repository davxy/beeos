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
#include "timer.h"
#include "kprintf.h"
#include "proc.h"


static void alarm_handler(void *data)
{
    struct task *t = (struct task *)data;

    list_delete(&t->alarm.plink);
    task_signal(t, SIGALRM);
}

unsigned int sys_alarm(unsigned int seconds)
{
    unsigned int left = 0;
    unsigned long now, when;
    struct timer_event *tm;

    tm = &current->alarm;
    if (tm->func != alarm_handler)
        timer_event_init(tm, alarm_handler, current, 0);

    when = tm->expires;
    now = timer_ticks;
    if (when > now)
        left = ticks_to_msecs(when-now)/1000;

    if (seconds != 0) {
        when = now + msecs_to_ticks(seconds*1000);
        timer_event_mod(tm, when);
        /* Also add to the process timers */
        if (list_empty(&tm->plink))
            list_insert_before(&current->timers, &tm->plink);
    } else {
        if (!list_empty(&tm->link))
            timer_event_del(tm);
        if (!list_empty(&tm->plink))
            list_delete(&tm->plink);
    }

    return left;
}
