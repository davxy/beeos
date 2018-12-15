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

#ifndef BEEOS_TIMER_H_
#define BEEOS_TIMER_H_

#include "list.h"
#include <stdint.h>
#include <time.h>


/* Clock ticks since system startup. */
extern clock_t timer_ticks;

/** Converts milliseconds to clock ticks. */
#define msecs_to_ticks(msecs) \
        (((unsigned long)(msecs) + (1000L / CLOCKS_PER_SEC) - 1) / (1000L / CLOCKS_PER_SEC))

#define ticks_to_msecs(ticks) \
        ((1000L / CLOCKS_PER_SEC) * (unsigned long)(ticks))


/** Timer event callback signature. */
typedef void (timer_event_t)(void *data);

/** Timer event structure. Represents an asynchrounous event. */
struct timer_event {
    struct list_link link;      /**< Link used when in the global queue. */
    struct list_link plink;     /**< Link for timers within the same process */
    timer_event_t    *func;     /**< Timer event function callback. */
    void             *data;     /**< User context data. */
    unsigned long    expires;   /**< Expiration time, in system ticks. */
};

/**
 * Initialize the timer event structure.
 *
 * @param tm        Timer event context.
 * @param fm        Timer event function callback.
 * @param data      Data pointer to be passed to the callback.
 * @param expires   Expiration time, in system ticks.
 */
void timer_event_init(struct timer_event *tm, timer_event_t *fn,
                      void *data, unsigned long expires);

/**
 * Adds a timer event to the timers queue.
 * The event will be fired after the delay specified with the init function.
 *
 * @param tm        Timer event context.
 */
void timer_event_add(struct timer_event *tm);

/**
 * Removes a timer event from the timers queue.
 * As a consequence prevents to let the event fires its action.
 *
 * @param tm        Timer event context.
 */
void timer_event_del(struct timer_event *tm);

/**
 * Adds a timer queue to the timers queue.
 * This function also specifies the expration time in ticks.
 * If the expration time is less than or equal the current 'timer_ticks'
 * value the event is immediatelly executed.
 *
 * @param tm        Timer event structure.
 * @param expires   Event expiration time, in system ticks.
 */
void timer_event_mod(struct timer_event *tm, unsigned long expires);

/**
 * Initialize the timer event queue.
 */
void timer_init(void);

/**
 * Timer wheel update.
 *
 * Eventually fires some asynchronous events.
 */
void timer_update(void);

#endif /* BEEOS_TIMER_H_ */
