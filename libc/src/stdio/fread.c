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
#include <unistd.h>

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t n;
    char *buf = (char *)ptr;
    int c;

    n = 0;
    while (n < size * nmemb) {
        if ((c = fgetc(stream)) == EOF)
            break;
        buf[n++] = (unsigned char)c;
        if (stream->bufmode == _IOLBF && c == '\n')
            break;
    }
    return n;
}
