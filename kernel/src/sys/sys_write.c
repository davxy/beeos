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
#include "dev.h"
#include "fs/vfs.h"
#include "proc.h"
#include <stddef.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

ssize_t sys_write(int fdn, const void *buf, size_t count)
{
    ssize_t n = 0;
    struct file *file;

    if (OPEN_MAX <= fdn || !current_task->fd[fdn].file)
        return -EBADF;

    file = current_task->fd[fdn].file;

    /* temporary */
    if (!file->inode)
        return -1;

    switch (file->inode->mode & S_IFMT) {
        case S_IFBLK:
        case S_IFCHR:
            n = dev_io(0, file->inode->dev, DEV_WRITE, file->offset, 
                       (void *)buf, count, NULL);
            break;
        case S_IFDIR:
            n = -EBADF;
            break;
        case S_IFREG:
        case S_IFIFO:
        case S_IFSOCK:
            n = fs_write(file->inode, buf, count, file->offset);
            break;
        default:
            n = -1;
            break;
    }

    if (n > 0)
        file->offset += n;
    return n;
}
