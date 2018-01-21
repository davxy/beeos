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
#include <sys/stat.h>

struct sb *devfs_sb_get(void);

int sys_mount(const char *source, const char *target,
              const char *filesystemtype, unsigned long mountflags,
              const void *data)
{
    int res = 0;
    struct inode *idst, *isrc;

    idst = fs_namei(target);

    if (strcmp(source, "dev") != 0)
        isrc = fs_namei(source);
    else
        isrc = devfs_sb_get()->root;

    isrc->sb->mnt = idst->sb;
    idst->sb = isrc->sb;
    idst->ops = isrc->ops;

    return res;
}
