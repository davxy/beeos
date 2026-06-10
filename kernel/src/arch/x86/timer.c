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
#include "io.h"
#include "isr.h"

/* Internal clock frequency is 1193180 Hz. */
#define TIMER_ARCH_HZ       1193180 /* Built-in timer max frequency */
/*
 * The value we send to the PIT is the value to divide it's input
 * clock (1193180 Hz) to get the required frequency.
 * The divisor must be small enough to fit into 16-bits.
 *
 * TIMER_ARCH_FREQ / freq < 65536 => frequency > 18,20
 */
#define TIMER_DIVISOR       (TIMER_ARCH_HZ / CLOCKS_PER_SEC)

#define TIMER_IO_DAT        0x40    /* Data port */
#define TIMER_IO_CMD        0x43    /* Command port */

#define TIMER_OPMODE        0x04    /* Mode 2, rate generator */
#define TIMER_ACCESS        0x30    /* 16bit, LSB first */


static void timer_handler(void)
{
    timer_ticks++;
    timer_update();
}

void timer_arch_init(void)
{
    uint8_t lo, hi;

    outb(TIMER_IO_CMD, TIMER_OPMODE | TIMER_ACCESS);

    /* Divisor has to be sent byte-wise, so split here into upper/lower bytes.*/
    lo = (uint8_t) TIMER_DIVISOR;
    hi = (uint8_t) (TIMER_DIVISOR >> 8 );

    /* Send the frequency divisor. */
    outb(TIMER_IO_DAT, lo);
    outb(TIMER_IO_DAT, hi);

    /* register the timer callback */
    isr_register_handler(ISR_TIMER, timer_handler);
}
