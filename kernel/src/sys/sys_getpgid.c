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
#include <sys/types.h>
#include <errno.h>

/*
 * Returns the PGID of the process specified by pid. If pid is zero, the
 * process ID of the calling process is used.  (Retrieving the PGID of a
 * process other than the caller is rarely necessary, and the POSIX.1
 * getpgrp() is preferred for that task.)
 */
pid_t sys_getpgid(pid_t pid)
{
    const struct task *t = NULL;
    const struct task *curr = current;

    if (pid == 0)
        pid = current->pid;
    do {
        if (curr->pid == pid) {
            t = curr;
            break;
        }
        curr = list_container(curr->tasks.next, struct task, tasks);
    } while (curr != current);
    return (t != NULL) ? t->pgid : -ESRCH;
}
