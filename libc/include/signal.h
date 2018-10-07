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

#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <sys/types.h>

#define SIGHUP      1
#define SIGINT      2
#define SIGQUIT     3
#define SIGILL      4
#define SIGTRAP     5
#define SIGABRT     6
#define SIGIOT      SIGABRT
#define SIGBUS      7
#define SIGFPE      8
#define SIGKILL     9
#define SIGUSR1     10
#define SIGSEGV     11
#define SIGUSR2     12
#define SIGPIPE     13
#define SIGALRM     14
#define SIGTERM     15
#define SIGSTKFLT   16
#define SIGCHLD     17
#define SIGCONT     18
#define SIGSTOP     19
#define SIGTSTP     20
#define SIGTTIN     21
#define SIGTTOU     22
#define SIGURG      23
#define SIGXCPU     24
#define SIGXFSZ     25
#define SIGVTALRM   26
#define SIGPROF     27
#define SIGWINCH    28
#define SIGIO       29
#define SIGPOLL     29
#define SIGPWR      30
#define SIGSYS      31
#define SIGUNUSED   SIGSYS

#define NSIG        (SIGUNUSED + 1)

typedef int sig_atomic_t;

typedef void (*sighandler_t)(int);

#define SIG_ERR ((sighandler_t)-1)
#define SIG_DFL ((sighandler_t) 0)
#define SIG_IGN ((sighandler_t) 1)

typedef unsigned long sigset_t;

#define sigemptyset(set) \
    (*(set) = 0)

#define sigfillset(set) \
    ((*(set) = ~(sigset_t)0), 0)

#define sigaddset(set, n) \
    ((0 < (n) && (n) < NSIG) ? ((*(set) |= (1 << (n))), 0) : -1)

#define sigdelset(set, n) \
    ((0 < (n) && (n) < NSIG) ? ((*(set) &= ~(1 << (n))), 0) : -1)

#define sigismember(set, n) \
    ((0 < (n) && (n) < NSIG) ? ((*(set) & (1 << (n))) ? 1 : 0) : -1)

#define sigisemptyset(set) \
    (*(set) == 0)

typedef struct siginfo {
    int si_signo;
    int si_errno;
    int si_code;
} siginfo_t;

struct sigaction {
    union {
        void (*sa_handler)(int);
        void (*sa_sigaction)(int, siginfo_t *, void *);
    } __sa_handler;
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};
#define sa_handler   __sa_handler.sa_handler
#define sa_sigaction __sa_handler.sa_sigaction

#define SIG_BLOCK       0
#define SIG_UNBLOCK     1
#define SIG_SETMASK     2


int sigaction(int sig, const struct sigaction *act,
              struct sigaction *oact);

sighandler_t signal(int signo, sighandler_t handler);

int kill(pid_t pid, int sig);

int raise(int sig);

int sigprocmask(int how, const sigset_t *set, sigset_t *oset);

int sigsuspend(const sigset_t *mask);

#endif /* _SIGNAL_H_ */
