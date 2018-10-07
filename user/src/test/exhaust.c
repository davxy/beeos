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
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void user_test()
{
}

void kernel_test()
{
    pid_t pid;

    while (1) {
        syscall(__NR_info);
        pid = fork();
        if (pid < 0) {
            printf("fork error\n");
            return;
        } else if (pid == 0) {
            struct timespec ts;
            printf("child %d forked by %d\n", getpid(), getppid());
            ts.tv_sec = 0;
            ts.tv_nsec = 200000000;
            nanosleep(&ts, NULL);
        } else {
            wait(NULL);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("%s n\n"
               "  - 1 user memory\n"
               "  - 2 kernel memory\n", argv[0]);
        return 1;
    }

    switch(atoi(argv[1])) {
    case 1:
        user_test();
        break;
    case 2:
        kernel_test();
        break;
    default:
        printf("not a valid test\n");
        break;
    }

    return 0;
}
