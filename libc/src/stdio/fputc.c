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

int fputc(int c, FILE *stream)
{
    if (stream->buf == NULL) {
        stream->buf = (char *)malloc(stream->bufsize);
        if (stream->buf == NULL)
            return EOF;
        stream->nr = 0;
        stream->nw = 0;
        stream->ptr = stream->buf;
    }
    if (stream->nr > 0) {
        lseek(stream->fd, SEEK_CUR, -stream->nr);
        stream->nr = 0;
        stream->nw = 0;
        stream->ptr = stream->buf; 
    }
    
    *stream->ptr++ = c;
    stream->nw++;
    if (stream->nw >= stream->bufsize ||
        (stream->bufmode == _IOLBF && (char)c == '\n') ||
        stream->bufmode == _IONBF) {
        if (fflush(stream) != 0)
            c = EOF;
    }
    return (unsigned char)c;
}
