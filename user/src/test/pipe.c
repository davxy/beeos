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
#include <stdio.h>
#include <sys/wait.h>

#define MAXLINE 32

int main(int argc, char *argv[])
{
    int n;
    int fd[2];
    pid_t pid;
    char line[MAXLINE];

    if (pipe(fd) < 0) {
        printf("pipe error\n");
        return 1;
    }

    if ((pid = fork()) < 0) {
        printf("fork error\n");
        return 1;
    }

    if (pid > 0) {
        printf("[parent] pid: %d, pgrp: %d\n", getpid(), getpgid(0));
        printf("[parent] Sleep for 5 seconds\n");
        sleep(5);
        printf("[parent] Wake-up\n");
        /* parent */
        printf("[parent] Close read-end (fd[0])\nn");
        close(fd[0]);
        printf("[parent] Write to pipe\n");
        write(fd[1], "hello world\n", 12);
        printf("[parent] Wait for child to exit\n");
        wait(NULL);
        printf("[parent] Exiting\n");
    } else {
        /* child */
        printf("[child] pid: %d, pgrp: %d\n", getpid(), getpgid(0));
        printf("[child] Close write-end (fd[1])\n");
        close(fd[1]);
        n = read(fd[0], line, MAXLINE);
        printf("[child] Read from pipe\n");
        write(1, line, n);
        printf("[child] Exiting\n");
    }

    return 0;
}
