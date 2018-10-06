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

static int environ_relocate(int envlen);

int putenv(char *string)
{
    char *p;
    int stringlen;
    int namelen;
    int i;

    stringlen = strlen(string);
    if (string == NULL || stringlen == 0 || (p = strchr(string, '=')) == NULL) {
        errno = EINVAL;
        return -1;
    }

    namelen = p - string;
    /* space for: environ null terminator, new string pointer and data */
    for (i = 0; environ[i] != NULL; i++) {
        if (strncmp(environ[i], string, namelen) == 0) {
            environ[i] = string;
            return 0;
        }
    }

    /* The item was not found, reallocate the environment in the heap */
    if (environ_relocate(i) < 0)
        return -1;
    environ[i] = string;
    environ[i+1] = NULL;
    return 0;
}


static int environ_relocate(int envlen)
{
    static int inheap = 0;
    char **envp;
    size_t newsiz;

    /* +2, one for the NULL and one for the new env variable */
    newsiz = (2 + envlen) * sizeof(char *);
    if (!inheap)
        envp = malloc(newsiz);
    else
        envp = realloc(environ, newsiz);

    if (envp == NULL) {
        errno = ENOMEM;
        return -1;
    }

    if (!inheap) {
        memcpy(envp, environ, envlen*sizeof(char *));
        inheap = 1;
    }

    environ = envp;
    return 0;
}
