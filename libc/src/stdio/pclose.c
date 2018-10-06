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
#include <errno.h>
#include <sys/wait.h>

extern pid_t *popen_childs;

int pclose(FILE *stream)
{
    int fd, status;
    pid_t pid;

    if (popen_childs == NULL) {
        errno = EINVAL;
        return -1;  /* popen() has never been called */
    }

    fd = fileno(stream);
    if ((pid = popen_childs[fd]) == 0) {
        errno = EINVAL;
        return -1;
    }

    popen_childs[fd] = 0;
    if (fclose(stream) == EOF)
        return -1;

    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR)
            return -1;  /* error other that EINTR from waitpid() */
    }
    return status;
}
