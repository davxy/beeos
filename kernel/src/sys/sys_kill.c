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
#include "kprintf.h"
#include <errno.h>

/*
 * POSIX.1  requires  that  if  a process sends a signal to itself,
 * and the sending thread does not have the signal blocked, and no
 * other thread has it unblocked or is waiting for it in sigwait(3),
 * at least one unblocked signal must be delivered to the sending
 * thread before the kill() returns.
 *
 * This is respected because the do_signal is invoked in the
 * global isr procedure before returning to user-space.
 */

int sys_kill(pid_t pid, int sig)
{
    struct task *t;

    if (sig <= 0 || sig > SIGUNUSED)
        return -EINVAL;

    t = current;
    do {
        if (t->pid == pid) {
            /* TODO: check for permissions */

            /* if sig is 0, only permissions are checked */
            if (sig != 0)
                task_signal(t, sig);
            break;
        }
        t = struct_ptr(t->tasks.next, struct task, tasks);
    } while (t != current);

    return 0;
}
