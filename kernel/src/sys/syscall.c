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

#include "sys.h"
#include "proc.h"
#include "isr.h"
#include "kprintf.h"
#include <unistd.h>


#define SYSCALLS_NUM    (__NR_info + 1)

static const void *syscalls[SYSCALLS_NUM] = {
    [__NR_exit]         = sys_exit,
    [__NR_fork]         = sys_fork,
    [__NR_read]         = sys_read,
    [__NR_write]        = sys_write,
    [__NR_mknod]        = sys_mknod,
    [__NR_open]         = sys_open,
    [__NR_close]        = sys_close,
    [__NR_lseek]        = sys_lseek,
    [__NR_dup]          = sys_dup,
    [__NR_dup2]         = sys_dup2,
    [__NR_waitpid]      = sys_waitpid,
    [__NR_execve]       = sys_execve,
    [__NR_getpid]       = sys_getpid,
    [__NR_getppid]      = sys_getppid,
    [__NR_getpgid]      = sys_getpgid,
    [__NR_setpgid]      = sys_setpgid,
    [__NR_tcgetpgrp]    = sys_tcgetpgrp,
    [__NR_tcsetpgrp]    = sys_tcsetpgrp,
    [__NR_getcwd]       = sys_getcwd,
    [__NR_sbrk]         = sys_sbrk,
    [__NR_nanosleep]    = sys_nanosleep,
    [__NR_fstat]        = sys_fstat,
    [__NR_sigaction]    = sys_sigaction,
    [__NR_sigreturn]    = sys_sigreturn,
    [__NR_sigsuspend]   = sys_sigsuspend,
    [__NR_kill]         = sys_kill,
    [__NR_sigprocmask]  = sys_sigprocmask,
    [__NR_pipe]         = sys_pipe,
    [__NR_chdir]        = sys_chdir,
    [__NR_alarm]        = sys_alarm,
    [__NR_mount]        = sys_mount,
    [__NR_getuid]       = sys_getuid,
    [__NR_setuid]       = sys_setuid,
    [__NR_getgid]       = sys_getgid,
    [__NR_setgid]       = sys_setgid,
    [__NR_clock]        = sys_clock,
    [__NR_info]         = sys_info,
};


/* TODO this is arch specific */
typedef uint32_t (* syscall_f)(uint32_t arg1, ...);


static void syscall_handler(void)
{
    struct isr_frame *ifr = current->arch.ifr;

    if (ifr->eax < SYSCALLS_NUM && syscalls[ifr->eax] != NULL) {
        ifr->eax = ((syscall_f)syscalls[ifr->eax])(
                ifr->ebx, ifr->ecx, ifr->edx,
                ifr->esi, ifr->edi, ifr->ebp);
    } else {
        kprintf("Warning: unknown syscall number (%d)\n", ifr->eax);
        ifr->eax = -1;
    }
}

void syscall_init(void)
{
    isr_register_handler(ISR_SYSCALL, syscall_handler);
}
