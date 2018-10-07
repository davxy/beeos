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


#include "tty.h"
#include "uart.h"  /* Temporary */
#include "dev.h"
#include "proc.h"
#include "screen.h"
#include "timer.h"
#include <errno.h>


#define TTYS_CONSOLE    4
#define TTYS_TOTAL      TTYS_CONSOLE

static struct tty_st tty_table[TTYS_TOTAL];
static struct screen scr_table[TTYS_TOTAL];
static unsigned int tty_curr;


static struct tty_st *tty_lookup(dev_t dev)
{
    struct tty_st *tty = NULL;
    unsigned int i;

    if (dev == DEV_TTY) {
        for (i = 0; i < TTYS_TOTAL; i++) {
            if (current->tty == tty_table[i].dev) {
                tty = &tty_table[i];
                break;
            }
        }
    } else {
        i = (minor(dev) == 0 || dev == DEV_CONSOLE) ? tty_curr : (minor(dev) - 1);
        if (i < TTYS_TOTAL)
            tty = &tty_table[i];
    }
    return tty;
}


pid_t tty_getpgrp(void)
{
    pid_t pgrp = -1;
    struct tty_st *tty;

    tty = tty_lookup(current->tty);
    if (tty != NULL)
        pgrp = tty->pgrp;
    return pgrp;
}


int tty_setpgrp(pid_t pgrp)
{
    int res = -1;
    struct tty_st *tty;

    tty = tty_lookup(current->tty);
    if (tty != NULL) {
        res = 0;
        tty->pgrp = pgrp;
    }
    return res;
}


void tty_change(unsigned int n)
{
    if (n < TTYS_CONSOLE) {
        tty_curr = n;
        scr_table[n].dirty = 1;
    }
}


dev_t tty_get(void)
{
    unsigned int i;
    dev_t dev = (dev_t)-1;

    for (i = 0; i < TTYS_TOTAL; i++) {
        if (tty_table[i].pgrp == 0 || tty_table[i].pgrp == current->pgid) {
            tty_table[i].pgrp = current->pgid;
            dev = tty_table[i].dev;
            tty_table[i].refs++;
            break;
        }
    }
    return dev;
}


void tty_put(dev_t dev)
{
    struct tty_st *tty;

    tty = tty_lookup(dev);
    if (tty != NULL) {
        if (tty->refs > 0) {
            tty->refs--;
            if (tty->refs == 0)
                tty->pgrp = 0;
        }
    }
}


static int tty_read_wait(dev_t dev, int couldblock)
{
    struct tty_st *tty;
    int c = -1;

    tty = tty_lookup(dev);
    if (tty == NULL)
        return -EINVAL;

    spinlock_lock(&tty->rcond.lock);

    while (tty->rpos >= tty->wpos && couldblock != 0) {
        tty->rpos = tty->wpos = 0;
        /* TODO: If BLOCKING file */
        cond_wait(&tty->rcond);
    }
    if (tty->rpos < tty->wpos)
        c = tty->rbuf[tty->rpos++];

    spinlock_unlock(&tty->rcond.lock);

    return c;
}


ssize_t tty_read(dev_t dev, void *buf, size_t size)
{
    ssize_t n = 0;
    int key;
    unsigned char *p = (unsigned char *)buf;

    while (n < size) {
        key = tty_read_wait(dev, (n == 0));
        if (key == 0 && n == 0) {
            /*
             * A single line contains zero: this is made by a VEOF
             * character (^D), that is, the input is closed.
             */
            break;
        } else if (key < 0 && n == 0) {
            /* At the moment, there is just nothing to read. */
            return -EAGAIN;
        } else if (key < -0 && n > 0) {
            /* Finished to read */
            break;
        } else {
            p[n] = key;
        }
        n++;
    }
    return n;
}


static int tty_putchar(dev_t dev, int c)
{
    unsigned int i;
    struct screen *scr;
    struct tty_st *tty;

    tty = tty_lookup(dev);
    if (tty == NULL)
        return -EINVAL;
    i = minor(tty->dev) - 1; /* Here is "safe" to do so */
    scr = &scr_table[i];

    screen_putchar(scr, (char)c);

    /* Useful for debug */
    uart_putchar(c);
    return 0;
}


ssize_t tty_write(dev_t dev, const void *buf, size_t n)
{
    ssize_t i;

    for (i = 0; i < n; i++) {
        if (tty_putchar(dev, ((const unsigned char *)buf)[i]) < 0)
            break;
    }
    return i;
}


/*
 * This function is usually called in interrupt context by the lower
 * level interrupt handler (e.g kbd driver).
 * TODO: Move most work as possible to the process context (tty_read).
 */
void tty_update(char c)
{
    const char *echo_buf = &c;
    size_t echo_siz = 1;
    struct tty_st *tty = &tty_table[tty_curr];

    spinlock_lock(&tty->rcond.lock);

    if (tty->wpos >= MAX_CANON)
        tty->wpos = MAX_CANON-1;

    if (tty->rpos > tty->wpos)
        tty->rpos = tty->wpos = 0;

    if (c == '\b') {
        if (tty->wpos > tty->rpos) {
            tty->wpos--;    /* will be eventually adjusted below */
            echo_buf = "\b \b";
            echo_siz = 3;
        } else {
            echo_siz = 0;
        }
    } else {
        tty->rbuf[tty->wpos++] = c;
        if (c == '\0' || c == '\n') {
            /* Wakeup all the processes waiting on this device */
            cond_signal(&tty->rcond);
        }
    }

    spinlock_unlock(&tty->rcond.lock);

    if ((tty->attr.c_lflag & ECHO) != 0 && echo_siz != 0)
        tty_write(tty->dev, echo_buf, echo_siz);
}


static void tty_attr_init(struct termios *termptr)
{
    termptr->c_iflag = BRKINT | ICRNL;
    termptr->c_oflag = 0;
    termptr->c_cflag = 0;
    termptr->c_lflag = ECHO | ECHOE | ECHOK | ECHONL | ICANON | ISIG;
    termptr->c_cc[VEOF] = 0x04;     /* ASCII EOT */
    termptr->c_cc[VEOL] = 0x00;     /* undefined */
    termptr->c_cc[VERASE] = 0x08;   /* ASCII BS */
    termptr->c_cc[VINTR] = 0x03;    /* ASCII ETX */
    termptr->c_cc[VKILL] = 0x00;    /* undefined */
    termptr->c_cc[VMIN] = 0x00;     /* undefined */
    termptr->c_cc[VQUIT] = 0x1C;    /* ASCII FS */
    termptr->c_cc[VSTART] = 0x00;   /* undefined */
    termptr->c_cc[VSUSP] = 0x00;    /* undefined */
    termptr->c_cc[VTIME] = 0x00;
}


static void tty_struct_init(struct tty_st *tty, dev_t dev)
{
    tty->dev = dev;
    tty->pgrp = 0;
    tty->rbuf[0] = 0;
    tty->rpos = 0;
    tty->wpos = 0;
    cond_init(&tty->rcond);
    tty_attr_init(&tty->attr);
}


static struct timer_event refresh_tm;

static void refresh_func(void)
{
    if (scr_table[tty_curr].dirty != 0)
        screen_update(&scr_table[tty_curr]);
    timer_event_mod(&refresh_tm, timer_ticks + msecs_to_ticks(25));
}


void tty_init(void)
{
    int i;

    for (i = 0; i < TTYS_CONSOLE; i++) {
        tty_struct_init(&tty_table[i], DEV_TTY1 + i);
        screen_init(&scr_table[i]);
    }
    tty_curr = 0;

    uart_init();

    timer_event_init(&refresh_tm, (timer_event_t *)refresh_func, NULL,
                     timer_ticks + msecs_to_ticks(100));
    timer_event_add(&refresh_tm);
}
