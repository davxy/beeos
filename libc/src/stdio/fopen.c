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

#include "FILE.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

FILE *fopen(const char *path, const char *mode)
{
    int fd;
    FILE *fp;
    int flags;

    /* The 'b' letter is ignored on all POSIX conforming systems. */
    if (!strcmp(mode, "r") || !strcmp(mode, "rb"))
        flags = O_RDONLY;
    else if (!strcmp(mode, "r+") || !strcmp(mode, "r+b")
            || !strcmp(mode, "rb+"))
        flags = O_RDWR;
    else if (!strcmp(mode, "w") || !strcmp(mode, "wb"))
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    else if (!strcmp(mode, "w+") || !strcmp(mode, "w+b")
            || !strcmp(mode, "wb+"))
        flags = O_RDWR | O_CREAT | O_TRUNC;
    else if (!strcmp(mode, "a") || !strcmp(mode, "ab"))
        flags = O_WRONLY | O_CREAT | O_APPEND;
    else if (!strcmp(mode, "a+") || !strcmp(mode, "a+b")
            || !strcmp(mode, "ab+"))
        flags = O_RDWR | O_CREAT | O_APPEND;
    else
    {
        errno = EINVAL;
        return NULL;
    }

    if ((fp = malloc(sizeof(*fp))) == NULL)
        return NULL;

    /* If O_CREAT is not specified then mode is ignored */
    if ((fd = open(path, flags, 0666)) < 0)
    {
        free(fp);
        return NULL;
    }
    fp->fd = fd;
    fp->flags = 0;
    return fp; 
}
