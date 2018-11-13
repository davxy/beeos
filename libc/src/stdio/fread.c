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

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t n, s;
    char *buf = (char *)ptr;
    int c;
    int stop = 0;

    n = 0;
    while (n < nmemb && stop == 0) {
        s = 0;
        while (s < size) {
            if ((c = fgetc(stream)) == EOF)
                return n;
            buf[s++] = (unsigned char)c;
            if (stream->bufmode == _IOLBF && c == '\n') {
                stop = 1;
                break;
            }
        }
        buf += size;
        n++;
    }
    return n;
}
