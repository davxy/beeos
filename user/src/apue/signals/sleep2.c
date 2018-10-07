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

#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static jmp_buf env_alrm;

static void sig_alrm(int signo)
{
    longjmp(env_alrm, 1);
}

unsigned int sleep2(unsigned int seconds)
{
    if (signal(SIGALRM, sig_alrm) == SIG_ERR)
        return seconds;
    if (setjmp(env_alrm) == 0) {
        alarm(seconds); /* Start the timer */
        pause();    /* Next caught signal wakes us up */
    }
    return alarm(0);
}

int main(int argc, char *argv[])
{
    int seconds;
    unsigned int left;

    if (argc < 2) {
        printf("usage: sleep <seconds>\n");
        return 1;
    }
    seconds = atol(argv[1]);

    printf("(%d) sleep for %d seconds\n", getpid(), seconds);
    if ((left = sleep2(seconds)) > 0)
        printf("%d seconds left\n", left);
    printf(">>> awake\n");
    return 0;
}
