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

#include "proc.h"
#include <sys/types.h>
#include <errno.h>

int sys_setgid(gid_t gid)
{
    if (current_task->egid == 0)
    {
        /* If gid is not root then, after this, it will be
         * impossible for the program to regain root privileges. */
        current_task->gid  = gid;
        current_task->egid = gid;
        current_task->sgid = gid;
    }
    else if (current_task->gid == gid
          || current_task->sgid == gid)
    {
        current_task->egid = gid;
    }
    else
    {
        errno = EPERM;
        return -1;
    }
    return 0;
}
