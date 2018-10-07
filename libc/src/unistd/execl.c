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

#include <unistd.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>

int execl(const char *path, const char *arg, ...)
{
    int argc;
    char *argv[ARG_MAX];
    va_list ap;
    char *next_arg;

    va_start(ap, arg);
    next_arg = (char *)arg;
    for (argc = 0; argc < ARG_MAX; argc++) {
        argv[argc] = next_arg;
        if (argv[argc] == NULL)
            break;
        next_arg = va_arg(ap, char *);
    }
    return execve(path, argv, environ);
}
