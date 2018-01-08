/*
 * Copyright (c) 2015-2017, Davide Galassi. All rights reserved.
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
#include <stdlib.h>
#include <stddef.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

void sigchld(int signo)
{
    int status;
    pid_t pid;

    if (signo != SIGCHLD)
        return;

    while (1)
    {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0)
            printf("[init-sig] Child %d exit status : %d\n", pid, status);
        else
        {
            if (pid < 0)
                printf("[init-sig] SIGCHLD with no childs\n");
            else
                printf("[init-sig] Child status already reaped\n");
            break;
        }
    }
}

#define NSH 2

pid_t spawn_shell(void)
{
    char *sh_argv[] = { "/bin/sh", NULL };
    char *sh_envp[] = { "SHELL=/bin/sh", "PATH=.:/bin:/sbin", NULL };
    pid_t pid;

    printf("Spawn shell\n");

    pid = fork();
    if (pid == 0) {
        if (execve(sh_argv[0], sh_argv, sh_envp) < 0)
            return -1;
    }
    return pid;
}

int main(int argc, char *argv[])
{
    int status;
    pid_t pid;
    pid_t sh_pid[NSH];
    int i;

    /* Create virtual console devices */
    for (i = 1; i <= 2; i++) {
        if (mknod("console", S_IFCHR | 0644, 0x0500 + i) < 0)
            return 1;
    }
    
    if (signal(SIGCHLD, sigchld) < 0)
        perror("signal");

    for (i = 0; i < 2; i++) {
        if ((sh_pid[i] = spawn_shell()) < 0)
            return -1;
    }

    while (1) {
        pid = wait(&status);
        for (i = 0; i < NSH; i++) {
            if (pid == sh_pid[i]) {
                spawn_shell();
                break;
            }
        }
    }

    return 0;
}
