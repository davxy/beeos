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
#include <errno.h>
#include <unistd.h>

int unsetenv(const char *name)
{
    size_t namelen;
    size_t i;

    namelen = strlen(name);
    if (name == NULL || namelen == 0 || strchr(name, '=') != NULL) {
        errno = EINVAL;
        return -1;
    }

    for (i = 0; environ[i]; i++) {
        if (strncmp(name, environ[i], namelen) == 0)
            break;
    }
    /* Eventually move higher variables down */
    for (; environ[i]; i++)
        environ[i] = environ[i+1];
    return 0;
}
