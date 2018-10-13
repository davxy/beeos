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

#include <stdio.h>
#include <unistd.h>
#include "FILE.h"

int fflush(FILE *stream)
{
    int res = 0;
    ssize_t n;
    char *p = stream->buf;

    while (stream->nw > 0) {
        if ((n = write(stream->fd, p, stream->nw)) < 0) {
            stream->flags |= FILE_FLAG_ERROR;
            res = EOF;
            break;
        }
        if (n > stream->nw)
            n = stream->nw; /* be defensive */
        stream->nw -= n;
        p += n;
    }
    stream->ptr = stream->buf;
    stream->nw = 0;
    stream->nr = 0;
    return res;
}
