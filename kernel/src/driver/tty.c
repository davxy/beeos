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

#include "tty.h"
#include "dev.h"
#include "proc.h"

void console_putchar(int c);
void uart_putchar(int c);
void uart_init(void);

#define TTYS_CONSOLE    4
#define TTYS_SERIAL     0
#define TTYS_TOTAL      (TTYS_CONSOLE + TTYS_SERIAL)

struct tty_st tty_table[TTYS_TOTAL];

int tty_read(dev_t dev, int couldblock)
{
    struct tty_st *tty = &tty_table[0];
    int c = -1;

    spinlock_lock(&tty->rcond.lock);

    while (tty->rpos >= tty->wpos && couldblock)
    {
        tty->rpos = tty->wpos = 0;
        /* TODO: If BLOCKING file */
        cond_wait(&tty->rcond);
    }
    if (tty->rpos < tty->wpos)
        c = tty->rbuf[tty->rpos++];

    spinlock_unlock(&tty->rcond.lock);

    return c;
}


void tty_putchar(int c)
{
//    if ((dev & 0xFF00) != (major(DEV_CONSOLE) << 8))
//        return;
    uart_putchar(c);
#ifndef __arm__
    console_putchar(c);
#endif
}

/// TODO: create a common double ended queue structure
ssize_t tty_read2(void *buf, size_t n)
{
    struct tty_st *tty = &tty_table[0];

    spinlock_lock(&tty->rcond.lock);
    while (tty->rpos >= tty->wpos)
    {
        tty->rpos = tty->wpos = 0;
        cond_wait(&tty->rcond);
    }
    if (tty->rpos < tty->wpos) 
    {
        if (tty->wpos - tty->rpos < n)
            n = tty->wpos - tty->rpos;
        memcpy(buf, &tty->rbuf[tty->rpos], n);
        tty->rpos += n;
    }
    spinlock_unlock(&tty->rcond.lock);
    return -1;
}

ssize_t tty_write(void *buf, size_t n)
{
    size_t i; 
    for (i = 0; i < n; i++)
        tty_putchar(((uint8_t *)buf)[i]);
    return (ssize_t)n;
}



/* 
 * This function is usually called in interrupt context by the lower
 * level interrupt handler (e.g kbd driver).
 * TODO: Move most work as possible to the process context (tty_read).
 */
void tty_update(char c)
{
    char *echo_buf = &c;
    size_t echo_siz = 1;
    struct tty_st *tty = &tty_table[0];

    spinlock_lock(&tty->rcond.lock);
    
    if (tty->wpos >= MAX_CANON)
        tty->wpos = MAX_CANON-1;
  
    if (tty->rpos > tty->wpos)
        tty->rpos = tty->wpos = 0;

    if (c == '\b')
    {
        tty->wpos--;    /* will be eventually adjusted below */
        echo_buf = "\b \b";
        echo_siz = 3;
    }
    else
    {
        tty->rbuf[tty->wpos++] = c;
        if (c == '\0' || c == '\n')
        {
            /* Wakeup all the processes waiting on this device */
            cond_signal(&tty->rcond);
        }
    }

    spinlock_unlock(&tty->rcond.lock);

    if (tty->attr.c_lflag & ECHO)
        dev_io(0, tty->dev, DEV_WRITE, 0, echo_buf, echo_siz, NULL);
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
    tty->rbuf[0] = 0;
    tty->rpos = 0;
    tty->wpos = 0;
    cond_init(&tty->rcond);
    tty_attr_init(&tty->attr);
}

void tty_init(void)
{
    int i;
    for (i = 0; i < TTYS_CONSOLE; i++)
        tty_struct_init(&tty_table[i], DEV_CONSOLE + i);

    uart_init();
}
