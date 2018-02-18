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

#include "panic.h"
#include "kprintf.h"
#include <stdarg.h>

extern void freeze(void);
extern void print_stack_trace(void);

void panic(const char *fmt, ...)
{
    va_list va;

    va_start(va, fmt);
    kprintf("*** kernel panic: ");
    kvprintf(fmt, va);
//    print_stack_trace();
    kprintf("\n***\n");
    freeze();
}
