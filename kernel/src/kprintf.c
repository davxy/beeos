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

#include "kprintf.h"
#include "dev.h"
#include <stdio.h>
#include <stddef.h>

#define KPRINTF_BUFSIZ  64

int kvprintf(const char *fmt, va_list arg)
{
    char str[KPRINTF_BUFSIZ];
    int n;

    n = vsnprintf(str, KPRINTF_BUFSIZ, fmt, arg);
    if (n < 0)
        return -1;
    /* Write to the first console. */
    dev_io (0, DEV_CONSOLE + 1, DEV_WRITE, 0, str, n, NULL);
    return n;
}

int kprintf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    return kvprintf(fmt, ap);
}
