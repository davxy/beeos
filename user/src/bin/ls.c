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
#include <stddef.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
    int i;
    struct dirent *entry;
    DIR *dirp;
    const char *dirname = ".";

    if (argc > 1)
        dirname = argv[1];

    if ((dirp = opendir(dirname)) == NULL)
        return -1;

    i = 0;
    while ((entry = readdir(dirp)) != NULL) {
        printf("%-10s ", entry->d_name);
        if (++i == 7) {
            printf("\n");
            i = 0;
        }
    }
    if (i != 0)
        printf("\n");
    closedir(dirp);
    return 0;
}
