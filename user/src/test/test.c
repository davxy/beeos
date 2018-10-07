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



int main(int argc, char *argv[])
{
    char *cargv[2] = { "/test/hello", NULL };
    pid_t pid;
    int status;
    int i = 0;
    struct timespec ts;

    ts.tv_sec = 0;
    ts.tv_nsec = 100000;
    while (1) {
        printf("==================\nloop begin\n");
        if ((pid = fork()) < 0) {
            perror("fork error");
            break;
        } else if (pid == 0) {
            exit(0);
            status = execve(cargv[0], cargv, NULL);
            if (status < 0)
                perror("execve error");
            exit(1);
        } else {
            printf("wait child (%d)\n", pid);
            pid = waitpid(pid, &status, 0);
            printf("child (%d) exit status %d\n", pid, status);
            if (status != 0)
                break;
        }

        printf("%d - goto sleep\n", i++);
        if (nanosleep(&ts, NULL) < 0) {
            perror("nanosleep error");
            break;
        }
    }
    return 0;
}
