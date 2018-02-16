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
#include <unistd.h>
#include <errno.h>
#include <limits.h>

void fs_file_free(struct file *file);

int sys_close(int fdn)
{
    struct file *file;

    /* Validate file descriptor */
    if (fdn < 0 || OPEN_MAX <= fdn || !current_task->fd[fdn].fil)
        return -EBADF;

    file = current_task->fd[fdn].fil;
    current_task->fd[fdn].fil = NULL;
    current_task->fd[fdn].flags = 0;

    file->ref--;
    if (file->ref == 0)
    {
        /* Wake up the other end, to allow EOF recv in user space */
        if (S_ISFIFO(file->dent->inod->mode))
            vfs_write(file->dent->inod, NULL, 0, 0);
        dput(file->dent);

        fs_file_free(file);
    }

    return fdn;
}
