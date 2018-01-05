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

#ifndef _BEEOS_TASK_H_
#define _BEEOS_TASK_H_

#include "list.h"
#include "fs/vfs.h"
#include "sync/cond.h"
#include "timer.h"
#include <stdint.h>
#include <limits.h>
#include <sys/types.h>
#include <signal.h>
#include "arch/x86/task.h"

#define TASK_RUNNING    1
#define TASK_SLEEPING   2
#define TASK_ZOMBIE     3
#define TASK_READY      4

#define SIGNALS_NUM     (SIGUNUSED+1)

/** Process structure. */
struct task
{
    struct task_arch    arch;           /**< Architecture specific data. */
    pid_t               pid;            /**< Process ID. */
    pid_t               pgid;           /**< Process group ID */
    uid_t               uid;            /**< Real user ID. */
    uid_t               euid;           /**< Effective user ID. */
    uid_t               suid;           /**< Saved used ID. */
    gid_t               gid;            /**< Real group ID. */
    gid_t               egid;           /**< Effective group ID. */
    gid_t               sgid;           /**< Saved group ID. */
    int                 state;          /**< Process state. */
    struct inode        *cwd;           /**< Current working directory. */
    struct fd           fd[OPEN_MAX];   /**< Open files. */  
    struct list_link    tasks;          /**< Tasks list link. */
    struct cond         chld_exit;      /**< Child exit condition */
    int                 counter;        /**< Remaining time slice for sched */
    int                 exit_code;      /**< Exit status */
    struct task         *pptr;          /**< Parent process */
    struct list_link    children;       /**< Children list (vertical) */
    struct list_link    sibling;        /**< Siblings list (horizontal) */
    uintptr_t           brk;            /**< Program break */
    sigset_t            sigpend;        /**< Pending signals */
    sigset_t            sigmask;        /**< Masked */
    struct sigaction    signals[SIGNALS_NUM];   /**< Signal handlers */
    struct list_link    timers;         /**< Process running timer events */
    struct timer_event  alarm;          /**< Alarm timer event (pre-allocated) */
};

struct task *task_create(void);
void task_delete(struct task *task);

int task_init(struct task *task);
void task_deinit(struct task *task);


int task_arch_init(struct task_arch *task);
void task_arch_deinit(struct task_arch *task);

void task_arch_switch(struct task_arch *curr, struct task_arch *next);


#endif /* _BEEOS_TASK_H_ */
