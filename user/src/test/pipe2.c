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
#include <string.h>

#define MAXLINE 100

int main(int argc, char *argv[])
{
    int n, i = 0;
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
        /* parent */
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        while (1) {
            printf("[%d] Hello World this is a long string", i++);
            usleep(100000);
        }
    } else {
        /* child */
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        while (1) {
            memset(line, 0, sizeof(line));
            n = read(STDIN_FILENO, line, MAXLINE-1);
            line[n] = 0; // add null terminator
            printf("%s\n", line);
        }
    }

    return 0;
}
