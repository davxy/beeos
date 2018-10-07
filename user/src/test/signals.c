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

#include <signal.h>
#include <stdio.h>

void handler(int signo)
{
    printf("SIG %d handler\n", signo);
}


int main(void)
{
    sigset_t set;
    struct sigaction act, oact;
    int status;

    status = sigemptyset(&set);
    if (status < 0)
        printf("error: sigemptyset\n");
    status = sigaddset(&set, SIGTSTP);
    if (status < 0)
        printf("error: sigaddset SIGTSTP\n");
    status = sigaddset(&set, 64);
    if (status < 0)
        printf("error: sigaddset 64\n");
    if (sigismember(&set, SIGTSTP))
        printf("SIGTSTP in set\n");
    if (sigismember(&set, SIGINT))
        printf("SIGINT in set\n");

    status = sigfillset(&set);
    if (status < 0)
        printf("error: sigfillset\n");

    act.sa_handler = handler;
    act.sa_mask = 0;
    act.sa_flags = 0;
    act.sa_restorer = NULL;
    sigaction(SIGINT, &act, &oact);

    raise(SIGINT);

    return 0;
}
