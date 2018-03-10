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

#include "sys.h"
#include "proc.h"
#include <errno.h>
#include <fcntl.h>

int sys_dup2(int oldfd, int newfd)
{
    int status;

    if (oldfd < 0 || oldfd >= OPEN_MAX || !current_task->fds[oldfd].fil)
        return -EBADF; /* Invalid file descriptor */

    if (oldfd == newfd)
        return oldfd;

    if (current_task->fds[newfd].fil != NULL) {
        status = sys_close(newfd);
        if (status < 0)
            return status;
    }

    current_task->fds[newfd] = current_task->fds[oldfd];
    current_task->fds[newfd].flags &= ~FD_CLOEXEC; /* Posix required */
    current_task->fds[newfd].fil->ref++;
    return newfd;
}

