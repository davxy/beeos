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
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

static void sig_alrm(int signo)
{
    /* nothing to do, just return to wake up the pause */
}

unsigned int sleep1(unsigned int seconds)
{
    if (signal(SIGALRM, sig_alrm) == SIG_ERR)
        return seconds;
    alarm(seconds); /* Start the timer */
    pause();        /* Next caught signal wakes us up */
    return alarm(0); /* Turn off timer, return unslept time */
}

int main(int argc, char *argv[])
{
    int seconds;
    unsigned int left;

    if (argc < 2) {
        printf("usage: sleep1 <seconds>\n");
        return 1;
    }
    seconds = atol(argv[1]);

    printf("(%d) sleep for %d seconds\n", getpid(), seconds);
    if ((left = sleep1(seconds)) > 0)
        printf("%d seconds left\n", left);
    printf(">>> Wake up\n");
    return 0;
}
