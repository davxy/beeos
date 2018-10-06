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
#include <string.h>
#include <unistd.h>
#include <errno.h>

char *getenv(const char *name)
{
    int namelen;
    char *value;
    int i;

    namelen = strlen(name);
    if (name == NULL || namelen == 0 || strchr(name, '=') != NULL) {
        errno = EINVAL;
        return NULL;
    }

    i = 0;
    value = NULL;
    while (environ[i] != NULL) {
        if (strncmp(name, environ[i], namelen) == 0) {
            value = environ[i] + namelen + 1;
            break;
        }
        i++;
    }
    return value;
}
