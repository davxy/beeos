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

void *fs_file_alloc(void);

int sys_open(const char *pathname, int flags, mode_t mode)
{
    int fdn;
    struct file *file;
    struct inode *inode;
    char buf[16];

    if (pathname == NULL)
        return -EINVAL;

    if (strcmp(pathname, "/dev/tty") == 0)
    {
        dev_t dev = tty_get();
        snprintf(buf, sizeof(buf), "/dev/tty%d", minor(dev));
        pathname = buf;
    }

    inode = namei(pathname);
    if (inode == NULL)
        return -ENOENT;

    for (fdn = 0; fdn < OPEN_MAX; fdn++)
        if (current_task->fd[fdn].file == NULL)
            break;
    if (fdn == OPEN_MAX)
        return -EMFILE; /* Too many open files. */

    // TODO : temporary just to allow to proceed
#if 0
    if (strcmp(pathname, "console") == 0)
    {
        inode = kmalloc(sizeof(struct inode), 0);
        memset(inode, 0, sizeof(*inode));
        inode->mode = S_IFCHR;
        inode->dev = tty_get();
        inode->ref = 1;
    }
    else
    {
#endif
        inode = namei(pathname);
        if (inode == NULL)
            return -ENOENT;
 //   }

    file = fs_file_alloc();
    if (!file)
        return -ENOMEM;

    file->refs = 1;
    file->offset = 0;
    file->inode = inode;

    current_task->fd[fdn].file = file;
    
    return fdn;
}
