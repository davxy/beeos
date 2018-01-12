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

#ifndef _BEEOS_TIMER_H_
#define _BEEOS_TIMER_H_

#include "list.h"
#include <stdint.h>


/* Clock timer frequency. */
extern unsigned long timer_freq;

/* Clock ticks since system startup. */
extern unsigned long timer_ticks;

/** Converts milliseconds to clock ticks. */
static inline unsigned long msecs_to_ticks(unsigned long msecs)
{
    //return (freq / 1000L) * msecs;
    return (msecs + (1000L / timer_freq) - 1) / (1000L / timer_freq);
}

static inline unsigned long ticks_to_msecs(unsigned long ticks)
{
    return (1000L / timer_freq) * ticks;
}

struct timer_event;

/** Timer event callback signature. */
typedef void (timer_event_t)(void *data);

/** Timer event structure. Represents an asynchrounous event. */
struct timer_event
{
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
 *
 * @param fraq      Required timer frequency in Hz (greather than 18).
 */
void timer_init(unsigned int frequency);

/**
 * Architecture dependent timer initialization.
 *
 * @param freq  Required timer frequency in Hz.
 */
void timer_arch_init(unsigned int freq);

/**
 * Timer wheel update. 
 *
 * Eventually fires some asynchronous events.
 */
void timer_update(void);

#endif /* _BEEOS_TIMER_H_ */

