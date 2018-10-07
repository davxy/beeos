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
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <limits.h>

/* Pointer to array allocated at run-time */
pid_t *popen_childs = NULL;

/* TODO: use sysconf to get max number of open files */

FILE *popen(const char *command, const char *type)
{
    int     i;
    int     pfd[2];
    pid_t   pid;
    FILE    *fp;

    /* only allow "r" or "w" */
    if ((type[0] != 'r' && type[0] != 'w') || type[1] != 0) {
        errno = EINVAL; /* required by POSIX */
        return NULL;
    }

    if (popen_childs == NULL) { /* first time through */
        /* allocate zeroed out array for child pids */
        if ((popen_childs = calloc(OPEN_MAX, sizeof(pid_t))) == NULL)
            return NULL;
    }

    if (pipe(pfd) < 0)
        return NULL;    /* errno set by pipe() */

    if ((pid = fork()) < 0) {
        return NULL;    /* errno set by fork() */
    } else if (pid == 0) {
        /* child */
        if (*type == 'r') {
            close(pfd[0]);
            if (pfd[1] != STDOUT_FILENO) {
                dup2(pfd[1], STDOUT_FILENO);
                close(pfd[1]);
            }
        } else {
            close(pfd[1]);
            if (pfd[0] != STDIN_FILENO) {
                dup2(pfd[0], STDIN_FILENO);
                close(pfd[0]);
            }
        }

        /* close all descriptors in chldpid[] */
        for (i = 0; i < OPEN_MAX; i++) {
            if (popen_childs[i] > 0)
                close(i);
        }

        execl("/bin/sh", "sh", "-c", command, NULL);
        _exit(127);
    }

    /* parent continues... */

    if (*type == 'r') {
        close(pfd[1]);
        if ((fp = fdopen(pfd[0], type)) == NULL)
            return NULL;
    } else {
        close(pfd[0]);
        if ((fp = fdopen(pfd[1], type)) == NULL)
            return NULL;
    }

    popen_childs[fileno(fp)] = pid; /* remember child pid for this fd */
    return fp;
}
