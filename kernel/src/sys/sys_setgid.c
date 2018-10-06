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

int sys_setgid(gid_t gid)
{
    if (current->egid == 0) {
        /* If gid is not root then, after this, it will be
         * impossible for the program to regain root privileges. */
        current->gid  = gid;
        current->egid = gid;
        current->sgid = gid;
    } else if (current->gid == gid || current->sgid == gid) {
        current->egid = gid;
    } else {
        errno = EPERM;
        return -1;
    }
    return 0;
}
