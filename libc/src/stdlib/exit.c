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
#include <stddef.h>

extern void (* _atexit_tab[ATEXIT_MAX])(void);

void exit(int status)
{
    int i;

    for (i = ATEXIT_MAX-1; i >= 0; i--) {
        if (_atexit_tab[i] != NULL)
            _atexit_tab[i]();
    }
    _Exit(status);
}
