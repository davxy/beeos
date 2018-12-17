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
#include "fs/devfs/devfs.h"
#include "proc.h"
#include <errno.h>
#include <sys/stat.h>


int sys_mount(const char *source, const char *target,
              const char *fs_type, unsigned long flags,
              const void *data)
{
    struct dentry *dst, *src;

    dst = named(target);
    if (dst == NULL)
        return -ENOENT;

    if (strcmp(fs_type, "dev") == 0)
        src = devfs_sb_get()->root;
    else
        src = named(source);

    return do_mount(dst, src);
}
