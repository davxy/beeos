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
#include <unistd.h>
#include <stddef.h>
#include <sys/wait.h>
#include <stdlib.h>

static void func2(void)
{
    while (1)
        printf("2");
}

static void func1(void)
{
    pid_t pid;

    if ((pid = fork()) < 0)
        return;
    else if (pid == 0)
        func2();

    while (1)
        printf("1");
}


int main(int argc, char *argv[])
{
    pid_t pid;
    int status;

    if ((pid = fork()) < 0) {
        perror("fork error");
        return 1;
    }
    else if (pid == 0) {
        func1();
    }

    printf("wait child (%d)\n", pid);
    pid = wait(&status);
    printf("child (%d) exit status %d\n", pid, status);

    return 0;
}
