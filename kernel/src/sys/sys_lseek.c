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
#include "proc.h"
#include <unistd.h>
#include <errno.h>
#include <limits.h>

off_t sys_lseek(int fd, off_t offset, int whence)
{
    struct file *file;
    off_t newoffset;

    if (fd < 0 || OPEN_MAX <= fd || !current_task->fds[fd].fil)
        return -EBADF;
    
    file = current_task->fds[fd].fil;
    switch (whence)
    {
        case SEEK_SET:
            newoffset = offset;
            break;
        case SEEK_CUR:
            newoffset = file->off + offset;
            break;
        case SEEK_END:
            newoffset = (off_t)file->dent->inod->size + offset;
            break;
        default:
            newoffset = -1;
            break;
    }
    
    if (newoffset < 0)
        return -EINVAL;
    file->off = newoffset;
    return file->off;
}
