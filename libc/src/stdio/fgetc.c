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

#include "FILE.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int _fillbuf(FILE *stream)
{
    if (stream->buf == NULL) {
        if ((stream->buf = (char *)malloc(stream->bufsize)) == NULL)
            return EOF;
    }
    stream->ptr = stream->buf;
    stream->nr = read(stream->fd, stream->ptr, stream->bufsize);
    if (--stream->nr < 0) {
        if (stream->nr == -1)
            stream->flags |= FILE_FLAG_EOF;
        else
            stream->flags |= FILE_FLAG_ERROR;
        stream->nr = 0;
        stream->nw = 0;
        return EOF;
    }
    return (unsigned char) *stream->ptr++;
}

int fgetc(FILE *stream)
{
    if (stream->nw > 0)
        fflush(stream);
    return (--(stream)->nr >= 0) ?
        (unsigned char)*stream->ptr++ : _fillbuf(stream);
}
