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
#include <errno.h>

int sys_fstat(int fd, struct stat *buf)
{
    const struct inode *inod;

    if (current->fds[fd].fil == NULL)
        return -EBADF;  /* Bad file descriptor */

    inod = current->fds[fd].fil->dent->inod;
    if (inod == NULL)
        return -ENOENT;

    buf->st_dev = inod->sb->dev;
    buf->st_ino = inod->ino;
    buf->st_mode = inod->mode;
    buf->st_nlink = 0; /* TODO */
    buf->st_uid = inod->uid;
    buf->st_gid = inod->gid;
    buf->st_rdev = inod->rdev;
    buf->st_size = inod->size;
    buf->st_atime = inod->atime;
    buf->st_mtime = inod->mtime;
    buf->st_ctime = inod->ctime;
    buf->st_blksize = 0;
    buf->st_blocks = 0;

    return 0;
}
