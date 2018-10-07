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

/**
 * @file    memmove.c
 *
 * @brief
 *
 * @author    dave
 * @date    Jul 12, 2014
 */

#include <string.h>


void *memmove(void *dst, const void *src, size_t n)
{
    char *d = (char *) dst;
    char *s = (char *) src;
    size_t i;

    /*
     * Depending on the memory start locations, copy may be direct or
     * reverse, to avoid overwriting before the relocation is done.
     */
    if (d < s) {
        for (i = 0; i < n; i++)
            d[i] = s[i];
    } else { /* s <= d */
        i = n;
        while (i-- > 0)
            d[i] = s[i];
    }
    return dst;
}
