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

#include "kprintf.h"
#include "dev.h"
#include "driver/tty.h"
#include <stdio.h>
#include <stddef.h>

#define KPRINTF_BUFSIZ  64

/*
 * Log to the first console.
 */
void kvprintf(const char *fmt, va_list arg)
{
    char str[KPRINTF_BUFSIZ];
    int n;

    n = vsnprintf(str, KPRINTF_BUFSIZ, fmt, arg);
    if (n > 0)
        tty_write(DEV_CONSOLE, str, n);
}

void kprintf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    kvprintf(fmt, ap);
    va_end(ap);
}
