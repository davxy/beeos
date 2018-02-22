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
#include "fs/vfs.h"
#include "proc.h"
#include <errno.h>

int sys_fstat(int fd, struct stat *buf)
{
    struct inode *inode;

    if (current_task->fds[fd].fil == NULL)
        return -EBADF;  /* Bad file descriptor */

    inode = current_task->fds[fd].fil->dent->inod;
    if (inode == NULL)
        return -ENOENT;

    buf->st_dev = inode->sb->dev;
    buf->st_ino = inode->ino;
    buf->st_mode = inode->mode;
    buf->st_nlink = 0; /* TODO */
    buf->st_uid = inode->uid;
    buf->st_gid = inode->gid;
    buf->st_rdev = inode->rdev;
    buf->st_size = inode->size;
    buf->st_atime = inode->atime;
    buf->st_mtime = inode->mtime;
    buf->st_ctime = inode->ctime;
    buf->st_blksize = 0;
    buf->st_blocks = 0;

    return 0;
}
