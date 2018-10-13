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
#include <stdlib.>

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
    int res = 0;

    if (stream->buf != NULL) {
        if (stream->cnt > 0)
            fflush(stream);
        free(stream->buf);
    }
    stream->bufmode = mode;
    stream->bufsize = size;
    stream->buf = buf;
    stream->ptr = buf;
    stream->cnt = 0;
}
