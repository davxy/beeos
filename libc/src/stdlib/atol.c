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


#include <stdlib.h>
#include <ctype.h>


long atol(const char *str)
{
    int i;
    int sign = +1;
    long num;

    for (i = 0; isspace(str[i]); i++)
        ;

    if (str[i] == '+') {
        i++;
    } else if (str[i] == '-') {
        sign = -1;
        i++;
    }

    for (num = 0; isdigit(str[i]); i++) {
        num = (num << 3)+(num << 1);    /* num *= 10 */
        num += (str[i] - '0');
    }

    if (sign == -1)
        num = -num;
    return num;
}
