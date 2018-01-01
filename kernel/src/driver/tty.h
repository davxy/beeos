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

#ifndef _BEEOS_DRIVER_TTY_H_
#define _BEEOS_DRIVER_TTY_H_

#include "sync/cond.h"
#include <sys/types.h>
#include <termios.h>


#define MAX_CANON   256

struct tty_st
{
    dev_t dev;              /* Associated device */
    pid_t pgrp;             /* Foreground process group */
    struct termios attr;    /* termios attributes */
    struct cond rcond;      /* Read conditional variable */
    unsigned int rpos;      /* Input line position read */
    unsigned int wpos;      /* Input line position write */
    char rbuf[MAX_CANON];   /* Canonical input line */
};

void tty_init(void);
int tty_read(dev_t dev, int couldblock);

void tty_putchar(int c);

ssize_t tty_write(void *buf, size_t n);

/* Write a character to tty */
void tty_update(char c);

pid_t tty_getpgrp(void);
int tty_setpgrp(pid_t pgrp);

#endif /* _BEEOS_DRIVER_TTY_H_ */
