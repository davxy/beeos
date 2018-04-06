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

#include "err.h"
#include <signal.h>
#include <stddef.h>

static volatile sig_atomic_t sigflag;
static sigset_t newmask, oldmask, zeromask;

static void sig_usr(int signo)
{
    sigflag = 1;
}

void sync_enter(void)
{
    (void)sigemptyset(&zeromask);
    (void)sigemptyset(&newmask);
    (void)sigaddset(&newmask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask))
        err_sys("sigprocmask error");
    if (signal(SIGUSR1, sig_usr) == SIG_ERR)
        err_sys("signal(SIGUSR1) error");
}

void sync_leave(void)
{
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
        err_sys("sigprocmask error");
}

void sync_wait(void)
{
    while (sigflag == 0)
        sigsuspend(&zeromask);
    sigflag = 0;
}

void sync_tell(pid_t pid)
{
    kill(pid, SIGUSR1);
}
