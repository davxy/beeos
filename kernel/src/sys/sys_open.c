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
#include "fs/vfs.h"
#include "proc.h"
#include "driver/tty.h"
#include <errno.h>
#include <string.h>
#include <fcntl.h>


int sys_open(const char *pathname, int flags, mode_t mode)
{
    int fdn;
    struct file *fil;
    struct dentry *dent;

    if (pathname == NULL)
        return -EINVAL;

    dent = named(pathname);
    if (dent == NULL)
        return -ENOENT;

    if (current->pid == current->pgid &&
        (flags & O_NOCTTY) == 0 &&
        strcmp(pathname, "/dev/tty") == 0) {
        current->tty = tty_get();
        if (current->tty < 0)
            return -EBUSY;
    }

    for (fdn = 0; fdn < OPEN_MAX; fdn++) {
        if (current->fds[fdn].fil == NULL)
            break;
    }
    if (fdn == OPEN_MAX)
        return -EMFILE; /* Too many open files. */

    fil = fs_file_alloc();
    if (fil == NULL)
        return -ENOMEM;

    fil->ref = 1;
    fil->off = 0;
    fil->mode = mode;
    fil->flags = (unsigned int)flags & ~O_CLOEXEC;
    fil->dent = dent;

    current->fds[fdn].fil = fil;
    current->fds[fdn].flags = (unsigned int)flags & O_CLOEXEC;

    return fdn;
}
