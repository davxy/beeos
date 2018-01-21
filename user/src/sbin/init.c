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
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define NTTY    4

#define DEFAULT_HOSTNAME    "localhost"
#define SHELL               "/bin/sh"
#define PATH                ".:/bin:/sbin"


#include <fcntl.h>

void dev_test(void)
{
    int fd, n,i ;
    char buf[64];

    for (i = 0; i < sizeof(buf); i++)
        buf[i] = i;

    fd = open("/dev/zero", O_RDONLY, 0);
    if (fd < 0)
        perror("open /dev/zero");

    n = read(fd, buf, sizeof(buf));
    if (n != sizeof(buf))
        perror("read error");

    for (i = 0; i < sizeof(buf); i++) {
        if (buf[i] != 0) {
            printf("/dev/zero not worked!!!\n");
            break;
        }
    }

    close(fd);
}

void dev_init(void)
{
    if (mknod("/dev/zero", S_IFCHR, makedev(0x01, 0x05)) < 0)
        perror("mknod /dev/zero");
    if (mknod("/dev/tty1", S_IFCHR, makedev(0x05, 0x01)) < 0)
        perror("mknod /dev/tty1");
    if (mknod("/dev/tty2", S_IFCHR, makedev(0x05, 0x02)) < 0)
        perror("mknod /dev/tty2");
    if (mknod("/dev/tty3", S_IFCHR, makedev(0x05, 0x03)) < 0)
        perror("mknod /dev/tty3");
    if (mknod("/dev/tty4", S_IFCHR, makedev(0x05, 0x04)) < 0)
        perror("mknod /dev/tty4");

    dev_test();
}


/* Before fork */
void env_init(void)
{
    char host[64];

    if (gethostname(host, sizeof(host)) < 0) {
        perror("gethostname");
        strcpy(host, DEFAULT_HOSTNAME);
    }

    if (setenv("HOSTNAME", host, 1) < 0)
        perror("setenv: HOSTNAME");
}

/* After fork */
void env_update(int i)
{
    char tty[32];

    snprintf(tty, sizeof(tty), "/dev/tty%d", i);

    if (setenv("SHELL", SHELL, 1) < 0)
        perror("setenv: SHELL");
    if (setenv("PATH", PATH, 1) < 0)
        perror("setenv: PATH");
    if (setenv("TTY", tty, 1) < 0)
        perror("setenv: TTY");
}

pid_t spawn_shell(int i)
{
    pid_t pid;

    pid = fork();
    if (pid == 0) {
        env_update(i);

        if (execl(SHELL, SHELL, NULL) < 0)
            return -1;
    }
    return pid;
}


/*
 * Collect death children status.
 */
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
    int status;
    pid_t pid;
    pid_t sh_pid[NTTY];
    int i;

    if (signal(SIGCHLD, sigchld) < 0)
        perror("signal");

    env_init();
    dev_init();

    for (i = 0; i < NTTY; i++) {
        if ((sh_pid[i] = spawn_shell(i + 1)) < 0)
            return -1;
    }

    while (1) {
        pid = wait(&status);
        for (i = 0; i < NTTY; i++) {
            if (pid == sh_pid[i]) {
                spawn_shell(i + 1);
                break;
            }
        }
    }

    return 0;
}
