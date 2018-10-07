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
#include <stdio.h>

int main(int argc, char *argv[])
{
    int i;

    if (argc < 2) {
        printf("usage: %s <shell command(s)>\n", argv[0]);
        return 1;
    }

    for (i = 1; i < argc; i++) {
        if (system(argv[i]) != 0)
            printf("Error executing %s\n", argv[i]);
    }
    return 0;
}
