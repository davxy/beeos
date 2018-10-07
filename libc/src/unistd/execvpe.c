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

#include <unistd.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static int namep(const char *name, char *path, size_t pathmax)
{
    char *envpath, *endp;
    int n;

    envpath = getenv("PATH");
    /* Scan paths and try to find a file with that name */
    while (envpath != NULL) {
        endp = strchr(envpath, ':');
        if (endp != NULL)
            n = endp - envpath;
        else
            n = strlen(envpath);
        if (pathmax < n + 2 + strlen(name))
            continue;
        memcpy(path, envpath, n);
        path[n++] = '/';
        strcpy(path+n, name);

        if (access(path, F_OK) == 0)
            return 0;
        if (!endp)
            break;
        envpath = endp+1;
    }
    errno = ENOENT;
    return -1;
}


int execvpe(const char *file, char *const argv[], char *const envp[])
{
    char cmd[PATH_MAX];

    /*
     * Check if the original command contains at least a '/'.
     * Otherwise a scan for the environment variable 'PATH' must
     * be done
     */
    if (!strchr(file, '/')) {
        if (namep(file, cmd, PATH_MAX) < 0)
            return -1;
    } else {
        if (strlen(file) >= PATH_MAX)
            return -1;
        strcpy(cmd, file);
    }
    return execve(cmd, argv, envp);
}
