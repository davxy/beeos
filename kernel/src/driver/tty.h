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

#ifndef BEEOS_DRIVER_TTY_H_
#define BEEOS_DRIVER_TTY_H_

#include "sync/cond.h"
#include <sys/types.h>
#include <termios.h>


#define MAX_CANON   256

struct tty_st {
    dev_t dev;              /* Associated device */
    pid_t pgrp;             /* Foreground process group */
    int refs;                /* References to this tty */
    struct termios attr;    /* termios attributes */
    struct cond rcond;      /* Read conditional variable */
    unsigned int rpos;      /* Input line position read */
    unsigned int wpos;      /* Input line position write */
    char rbuf[MAX_CANON];   /* Canonical input line */
};

void tty_init(void);

ssize_t tty_read(dev_t dev, void *buf, size_t size);

ssize_t tty_write(dev_t dev, const void *buf, size_t n);

/* Write a character to tty */
void tty_update(char c);

void tty_change(unsigned int n);

pid_t tty_getpgrp(void);

int tty_setpgrp(pid_t pgrp);

dev_t tty_get(void);

void tty_put(dev_t dev);


#endif /* BEEOS_DRIVER_TTY_H_ */
