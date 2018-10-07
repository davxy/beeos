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
#include <signal.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>

int system(const char *cmd)
{
    pid_t pid;
    int status;
    struct sigaction ignore, saveintr, savequit;
    sigset_t chldmask, savemask;

    if (cmd == NULL) {
        errno = EINVAL;
        return -1;
    }

    /* ignore SIGINT and SIGQUIT */
    ignore.sa_handler = SIG_IGN;
    (void)sigemptyset(&ignore.sa_mask);
    ignore.sa_flags = 0;
    if (sigaction(SIGINT, &ignore, &saveintr) < 0)
        return -1;
    if (sigaction(SIGQUIT, &ignore, &savequit) < 0)
        return -1;

    /* now block SIGCHLD */
    (void)sigemptyset(&chldmask);
    (void)sigaddset(&chldmask, SIGCHLD);
    if (sigprocmask(SIG_BLOCK, &chldmask, &savemask) < 0)
        return -1;

    if ((pid = fork()) < 0)
        status = -1;    /* probably out of process */

    if (pid == 0) { /* Child */
        /* Restore previous signal actions & reset signal mask */
        sigaction(SIGINT, &saveintr, NULL);
        sigaction(SIGQUIT, &savequit, NULL);
        sigprocmask(SIG_SETMASK, &savemask, NULL);
        execlp("sh", "sh", "-c", cmd, NULL);
        _exit(127); /* exec error */
    }

    /*
     * The SIGCHLD is blocked so that we are able to retrive
     * the status and not be overrun by an eventually SIGCHLD
     * user handler that, in the worst case, can call the
     * waitpid beefore us.
     */
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            status = -1;
            break;
        }
    }

    /* Restore previous signal actions and reset signal mask */
    if (sigaction(SIGINT, &saveintr, NULL) < 0)
        return -1;
    if (sigaction(SIGQUIT, &savequit, NULL) < 0)
        return -1;
    if (sigprocmask(SIG_SETMASK, &savemask, NULL) < 0)
        return -1;
    return status;
}
