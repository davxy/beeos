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
#include <unistd.h>
#include <errno.h>
#include <limits.h>


int sys_close(int fd)
{
    struct file *fil;

    /* Validate file descriptor */
    if (fd < 0 || OPEN_MAX <= fd || !current->fds[fd].fil)
        return -EBADF;

    fil = current->fds[fd].fil;
    current->fds[fd].fil = NULL;
    current->fds[fd].flags = 0;

    fil->ref--;
    if (fil->ref == 0) {
        /* Wake up the other end, to allow EOF recv in user space */
        if (S_ISFIFO(fil->dent->inod->mode))
            (void)vfs_write(fil->dent->inod, NULL, 0, 0);
        dput(fil->dent);

        fs_file_free(fil);
    }
    return 0;
}
