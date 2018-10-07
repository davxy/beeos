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

/* libc */
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
/* libu */
#include <sync.h>
#include <err.h>

static void charatatime(char *str)
{
    char *ptr;
    int c;

    for (ptr = str; (c = *ptr++) != 0; ) {
        putc(c, stdout);
        usleep(100000);
    }
}

int main(void)
{
    pid_t pid;

    sync_enter();

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        printf("[child] wait parent\n");
        sync_wait();
        printf("[child] continue\n");
        charatatime("OUTPUT FROM CHILD\n");
        sleep(3);
        sync_tell(getppid());
    } else {
        sleep(3);
        charatatime("OUTPUT FROM PARENT\n");
        printf("[parent] wakeup child\n");
        sync_tell(pid);
        printf("[parent] wait child\n");
        sync_wait();
        printf("[parent] continue\n");
        wait(NULL);
    }
    sync_leave();

    return 0;
}
