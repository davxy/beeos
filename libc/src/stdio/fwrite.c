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

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t n, s;
    const char *buf = (const char *)ptr;

    n = 0;
    while (n < nmemb) {
        s = 0;
        while (s < size) {
            if (fputc(buf[s], stream) == EOF)
                return n;
            s++;
        }
        buf += size;
        n++;
    }
    return n;
}
