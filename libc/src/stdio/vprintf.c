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

/**
 * @file vprintf.c
 *
 * @brief
 *
 * @author     Davide Galassi
 * @date      Jul 15, 2014
 *
 *
 */


#include "FILE.h"
#include <stdio.h>
#include <unistd.h>

int vprintf(const char *format, va_list ap)
{
    int written;
    int tot;
    int rtot;
    char buf[BUFSIZ];
    char *ptr = buf;

    rtot = vsnprintf(buf, BUFSIZ, format, ap);
    tot = rtot;
    while (tot > 0) {
        written = write(stdout->fd, ptr, tot);
        if (written < 0)
            break;
        tot -= written;
        ptr += written;
    }

    return rtot;
}
