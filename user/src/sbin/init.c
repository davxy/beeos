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

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;
    char *sh_argv[] = { "/bin/sh", NULL };
    char *sh_envp[] = { "SHELL=/bin/sh", "PATH=.:/bin:/sbin", NULL };

    if (mknod("console", S_IFCHR | 0644, 0x0501) < 0)
        return 1;

    if (signal(SIGCHLD, sigchld) < 0)
        perror("signal");

    while (1)
    {
        pid = wait(&status);
        if (pid < 0)
        {
            /* exhausted the children, spawn a shell */
            pid = fork();
            if (fork < 0)
                return 1;
            
            if (pid == 0)
            {
                if (execve(sh_argv[0], sh_argv, sh_envp) < 0)
                    return 1;
            }
        }
        else
            printf("[init] child (%d) exit status: %d\n", pid, status);
    }

    return 0;
}
