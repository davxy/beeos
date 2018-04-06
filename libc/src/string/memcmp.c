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

#include <string.h>

int memcmp(const void *s1, const void *s2, size_t n)
{
    const char *a = (const char *) s1;
    const char *b = (const char *) s2;
    size_t i;
    int res = 0;

    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            res = (int)a[i] - b[i];
            break;
        }
    }
    return res;
}
