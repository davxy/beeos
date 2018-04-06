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

#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

void spanchild(int sleept)
{
    pid_t pid;

    if ((pid = fork()) < 0) {
        perror("fork error");
        exit(1);
    }

    if (pid == 0) {
        printf("[child] (pid: %d, ppid: %d) SLEEP(%d)\n",
                getpid(), getppid(), sleept);
        sleep(sleept);
        printf("[child] (pid: %d, ppid: %d) WAKEUP\n",
                getpid(), getppid());
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    spanchild(20);
    spanchild(10);
    sleep(3);
    printf("[parent] exit\n");
    return 0;
}
