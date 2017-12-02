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
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

#define CMD_MAX 64

static int interactive(void);
static int execute(int argc, char *argv[]);

static pid_t fgpid = -1;
static int fgterm;

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
        {
            //printf("[sh-sig] child %d, exit status %d, fgpid %d\n", 
            //        pid, status, fgpid);
            if (pid == fgpid)
                fgterm = 1;
        }
        else
        {
            //if (pid < 0)
            //   printf("[sh-sig] no childs\n");
            //else
            //    printf("[sh-sig] no childs terminated\n");
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    int status;
    sigset_t mask;
    
    /* Be sure that SIGCHLD is unblocked */
    (void)sigemptyset(&mask);
    (void)sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);

    if (signal(SIGCHLD, sigchld) < 0)
        perror("signal");

    if (argc > 2 && strcmp(argv[1], "-c") == 0)
        status = execute(argc-2, &argv[2]);
    else
        status = interactive();
    
    return status;
}

static int interactive(void)
{
    int fd;
    char cmd[CMD_MAX];
    char *argv[20];
    int argc;

    fd = open("console", O_RDWR, 0); /* stdin */
    if (fd < 0)
        return -1;
    dup(0); /* stdout */
    dup(0); /* stderr */

    while (1)
    {
        printf("$ ");
        if (fgets(cmd, CMD_MAX, stdin))
        {
            argc = 0;
            argv[argc++] = strtok(cmd, " ");
            while ((argv[argc++] = strtok(NULL, " ")) != NULL);
            argc--;
            execute(argc, argv);
        }
    }
    return 0;
}

static int execute(int argc, char *argv[])
{
    sigset_t zeromask, newmask, oldmask;
    pid_t pid;
    int status;
    char *cmd;
    int bg = 0;

    cmd = argv[0];

    /* check built-in commands first */
    if (strcmp(cmd, "exit") == 0)
        exit(0);
    else if (strcmp(cmd, "cd") == 0)
    {
        if ((status = chdir(argv[1])) < 0)
            printf("sh: cd: %s\n", strerror(errno));
    }
    else
    {
        (void)sigemptyset(&zeromask);
        (void)sigemptyset(&newmask);
        (void)sigaddset(&newmask, SIGCHLD);
        sigprocmask(SIG_BLOCK, &newmask, &oldmask);

        if (argc > 1 && *argv[argc-1] == '&')
        {
            argc--;
            bg = 1;
        }

        fgterm = 0;
        fgpid = pid = fork();
        if (pid < 0)
            printf("fork error\n");
        else if (pid == 0)
        {
            status = execvpe(cmd, argv, environ);
            if (status < 0)
            {
                printf("sh: %s: %s\n", cmd, strerror(errno));
                status = 1;
            }
            exit(status);
        }
        else if (!bg)
        {
            while (!fgterm)
            {
                //printf("Suspend, wait for %d\n", pid);
                sigsuspend(&zeromask);
                //printf("Resumed\n");
            }
        }

        sigprocmask(SIG_SETMASK, &oldmask, NULL);
    }
    return status;
}
