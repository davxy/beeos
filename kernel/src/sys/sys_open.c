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

#include "fs/vfs.h"
#include "proc.h"
#include "dev.h"
#include "kmalloc.h"
#include "driver/tty.h"
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>


void *fs_file_alloc(void);

int sys_open(const char *pathname, int flags, mode_t mode)
{
    int fdn;
    struct file *file;
    struct dentry *dentry;
    char buf[16];

    if (pathname == NULL)
        return -EINVAL;

    if (strcmp(pathname, "/dev/tty") == 0)
    {
        dev_t dev = tty_get();
        snprintf(buf, sizeof(buf), "/dev/tty%d", minor(dev));
        pathname = buf;
    }

    dentry = named(pathname);
    if (dentry == NULL)
        return -ENOENT;

    for (fdn = 0; fdn < OPEN_MAX; fdn++)
        if (current_task->fd[fdn].fil == NULL)
            break;
    if (fdn == OPEN_MAX)
        return -EMFILE; /* Too many open files. */

    file = fs_file_alloc();
    if (!file)
        return -ENOMEM;

    file->ref = 1;
    file->off = 0;
    file->mode = mode;
    file->flags = flags & ~O_CLOEXEC;
    file->dent = dentry;
    dget(dentry);

    current_task->fd[fdn].fil = file;
    current_task->fd[fdn].flags = flags & O_CLOEXEC;

    return fdn;
}
