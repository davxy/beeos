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
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <errno.h>

#define NTTY    4

#define DEFAULT_HOST        "localhost"
#define DEFAULT_USER        "guest"
#define SHELL               "/bin/sh"
#define PATH                "/bin:/sbin"

struct dev_desc {
    const char *name;
    mode_t      type;
    dev_t       dev;
};

static const struct dev_desc devs[] = {
    { "/dev/zero",    S_IFCHR, makedev(0x01, 0x05) },
    { "/dev/tty",     S_IFCHR, makedev(0x05, 0x00) },
    { "/dev/console", S_IFCHR, makedev(0x05, 0x01) },
    { "/dev/tty1",    S_IFCHR, makedev(0x04, 0x01) },
    { "/dev/tty2",    S_IFCHR, makedev(0x04, 0x02) },
    { "/dev/tty3",    S_IFCHR, makedev(0x04, 0x03) },
    { "/dev/tty4",    S_IFCHR, makedev(0x04, 0x04) },
    { "/dev/mem",     S_IFBLK, makedev(0x01, 0x01) },
    { "/dev/kmem",    S_IFCHR, makedev(0x01, 0x02) },
    { "/dev/random",  S_IFCHR, makedev(0x01, 0x08) },
    { "/dev/urandom", S_IFCHR, makedev(0x01, 0x09) },
    { "/dev/initrd",  S_IFBLK, makedev(0x01, 0xFA) },
};
#define NDEVS (sizeof(devs)/sizeof(*devs))

void dev_init(void)
{
    int i;

    if (mount("dev", "/dev", "dev", 0, NULL) < 0) {
        perror("mount of dev fs failure");
        return;
    }
    for (i = 0; i < NDEVS; i++) {
        if (mknod(devs[i].name, devs[i].type, devs[i].dev) < 0)
            printf("mknod %s error: %s", devs[i].name, strerror(errno));
    }
}


/* Before fork */
void env_init(void)
{
    char host[64];

    if (gethostname(host, sizeof(host)) < 0) {
        perror("gethostname");
        strcpy(host, DEFAULT_HOST);
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
    if (setenv("USER", DEFAULT_USER,1 ) < 0)
        perror("setenv: USER");
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
    while (1) {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0) {
            printf("[init-sig] Child %d exit status : %d\n", pid, status);
        } else {
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
