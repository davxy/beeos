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

#ifndef _BEEOS_SYS_H_
#define _BEEOS_SYS_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>


void sys_exit(int status);
pid_t sys_fork(void);

ssize_t sys_read(int fd, void *buf, size_t count);
ssize_t sys_write(int fd, const void *buf, size_t count);

int sys_mknod(const char *pathname, mode_t mode, dev_t dev);
int sys_open(const char *pathname, int flags, mode_t mode);
int sys_close(int fd);
off_t sys_lseek(int fd, off_t offset, int whence);
int sys_dup(int oldfd);
int sys_dup2(int oldfd, int newfd);

pid_t sys_waitpid(pid_t pid, int *wstatus, int options);

pid_t sys_getpid(void);
pid_t sys_getppid(void);

char *sys_getcwd(char *buf, size_t size);

uid_t sys_getuid(void);
gid_t sys_getgid(void);

int sys_setuid(uid_t uid);
int sys_setgid(gid_t gid);

int sys_execve(const char *path, const char *argv[], const char *envp[]);

void *sys_sbrk(intptr_t incr);

int sys_nanosleep(const struct timespec *req, struct timespec *rem);

int sys_fstat(int fd, struct stat *buf);

int sys_info(int type);

int sys_sigaction(int sig, const struct sigaction *act,
        struct sigaction *oact);

int sys_sigreturn(void);

int sys_sigsuspend(const sigset_t *mask);

int sys_kill(pid_t pid, int sig);

int sys_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

int sys_pipe(int pipefd[2]);

int sys_chdir(const char *path);

unsigned int sys_alarm(unsigned int seconds);

void syscall_init(void);


#endif /* _BEEOS_SYS_H_ */
