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

#ifndef _UNISTD_H_
#define _UNISTD_H_

#ifndef __ASSEMBLER__
#include <sys/types.h>
#include <sys/stat.h>
#endif

#define __NR_exit           1
#define __NR_fork           2
#define __NR_read           3
#define __NR_write          4
#define __NR_open           5
#define __NR_close          6
#define __NR_mknod          7
#define __NR_waitpid        8
#define __NR_creat          9
#define __NR_dup            10
#define __NR_dup2           11
#define __NR_execve         12
#define __NR_lseek          13
#define __NR_time           14
#define __NR_getpid         15
#define __NR_setpgid        16
#define __NR_getpgid        17
#define __NR_tcgetpgrp      18
#define __NR_tcsetpgrp      19
#define __NR_fstat          20
#define __NR_sbrk           21
#define __NR_nanosleep      22
#define __NR_sigaction      23
#define __NR_sigreturn      24
#define __NR_sigprocmask    25
#define __NR_sigsuspend     26
#define __NR_kill           27
#define __NR_pipe           28
#define __NR_chdir          29
#define __NR_alarm          30
#define __NR_mount          31
#define __NR_getppid        32
#define __NR_getcwd         33
#define __NR_getuid         34
#define __NR_getgid         35
#define __NR_setuid         36
#define __NR_setgid         37
#define __NR_clock          38
/* Custom info syscall */
#define __NR_info           39


#define STDIN_FILENO        0
#define STDOUT_FILENO       1
#define STDERR_FILENO       2

/* Values for the 'whence' argument to lseek */
#define SEEK_SET            0
#define SEEK_CUR            1
#define SEEK_END            2

/* Values for the second argument to access.
 * These may be OR'd together.  */
#define F_OK                0       /* Test for existence */
#define X_OK                1       /* Test for execute permission */
#define W_OK                2       /* Test for write permission */
#define R_OK                4       /* Test for read permission */


#ifndef __ASSEMBLER__

extern char **environ;  /**< Pointer to the environment strings */


long syscall(long nr, ...);


static inline void _exit(int status)
{
    syscall(__NR_exit, status);
}

static inline pid_t fork(void)
{
    return syscall(__NR_fork);
}

static inline ssize_t read(int fd, void *buf, size_t count)
{
    return syscall(__NR_read, fd, buf, count);
}

static inline ssize_t write(int fd, const void *buf, size_t count)
{
    return syscall(__NR_write, fd, buf, count);
}

static inline int mknod(const char *pathname, mode_t mode, dev_t dev)
{
    return syscall(__NR_mknod, pathname, mode, dev);
}

static inline int open(const char *pathname, int flags, mode_t mode)
{
    return syscall(__NR_open, pathname, flags, mode);
}

static inline int close(int fd)
{
    return syscall(__NR_close, fd);
}

static inline int lseek(int fd, off_t offset, int whence)
{
    return syscall(__NR_lseek, fd, offset, whence);
}

static inline int dup(int oldfd)
{
    return syscall(__NR_dup, oldfd);
}

static inline int dup2(int oldfd, int newfd)
{
    return syscall(__NR_dup2, oldfd, newfd);
}

static inline pid_t waitpid(pid_t pid, int *wstatus, int options)
{
    return syscall(__NR_waitpid, pid, wstatus, options);
}

static inline int execve(const char *file, char *const argv[],
        char *const envp[])
{
    return syscall(__NR_execve, file, argv, envp);
}

int execl(const char *path, const char *arg, ... /* (char *)NULL */);

int execlp(const char *path, const char *arg, ... /* (char *)NULL */);

int execv(const char *path, char *const argv[]);

int execvpe(const char *file, char *const argv[], char *const envp[]);

static inline char *getcwd(char *buf, size_t size)
{
    char *ret = buf;

    if (syscall(__NR_getcwd, buf, size) < 0)
        ret = (void *)0;
    return ret;
}

static inline pid_t getpid(void)
{
    return syscall(__NR_getpid);
}

static inline pid_t getppid(void)
{
    return syscall(__NR_getppid);
}

static inline int setpgid(pid_t pid, pid_t pgid)
{
    return syscall(__NR_setpgid, pid, pgid);
}

static inline pid_t getpgid(pid_t pid)
{
    return syscall(__NR_getpgid, pid);
}

static inline pid_t tcgetpgrp(int fd)
{
    return syscall(__NR_tcgetpgrp, fd);
}

static inline int tcsetpgrp(int fd, pid_t pgrp)
{
    return syscall(__NR_tcsetpgrp, fd, pgrp);
}

static inline int fstat(int fdn, struct stat *buf)
{
    return syscall(__NR_fstat, fdn, buf);
}

static inline void *sbrk(intptr_t incr)
{
    return (void *)syscall(__NR_sbrk, incr);
}

static inline int nanosleep(const struct timespec *req, struct timespec *rem)
{
    return syscall(__NR_nanosleep, req, rem);
}

static inline int pipe(int pipefd[2])
{
    return syscall(__NR_pipe, pipefd);
}

static inline int chdir(const char *path)
{
    return syscall(__NR_chdir, path);
}


static inline unsigned int sleep(unsigned int seconds)
{
    struct timespec req, rem;
    req.tv_sec = seconds;
    req.tv_nsec = 0;
    if (nanosleep(&req, &rem) < 0)
        return seconds;
    return rem.tv_sec + rem.tv_nsec/1000000000;
}

static inline int usleep(unsigned int usec)
{
    struct timespec req, rem;
    req.tv_sec = usec / 1000000;
    req.tv_nsec = (usec % 1000000) * 1000;
    if (nanosleep(&req, &rem) < 0)
        return -1;
    return 0;
}

int access(const char *path, int amode);

int pause(void);

int gethostname(char *name, size_t len);

static inline unsigned int alarm(unsigned int seconds)
{
    return syscall(__NR_alarm, seconds);
}


static inline uid_t getuid(void)
{
    return syscall(__NR_getuid);
}

static inline int setuid(uid_t uid)
{
    return syscall(__NR_setuid, uid);
}

static inline gid_t getgid(void)
{
    return syscall(__NR_getgid);
}

static inline int setgid(gid_t gid)
{
    return syscall(__NR_setgid, gid);
}


#endif /* _ASSEMBLER_ */

#endif /* _UNISTD_H_ */
