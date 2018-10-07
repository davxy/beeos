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

int setenv(const char *name, const char *value, int overwrite)
{
    int namelen;
    char *oldval;
    char *var;

    namelen = strlen(name);
    if (name == NULL || namelen == 0 || strchr(name, '=') != NULL) {
        errno = EINVAL;
        return -1;
    }

    oldval = getenv(name);
    if (oldval != NULL) {
        if (!overwrite) {
            errno = EINVAL;
            return -1;
        }
        if (strlen(value) <= strlen(oldval)) {
            strcpy(oldval, value);
            return 0;
        } else {
            /* It doesn't fit we must remove the old variable */
            if (unsetenv(name) < 0)
                return -1;
        }
    }

    /* At this point the variable is not set */
    var = malloc(strlen(name) + strlen(value) + 2);
    if (var == NULL)
        return -1;
    strcpy(var, name);
    strcat(var, "=");
    strcat(var, value);

    if (putenv(var) != 0) {
        free(var);
        return -1;
    }
    return 0;
}
