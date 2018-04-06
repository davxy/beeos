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
#include <sys/stat.h>

int sys_chdir(const char *path)
{
    struct dentry *dent, *tmp;
    const struct inode *inod;

    dent = named(path);
    if (dent == NULL)
        return -ENOENT;
    inod = dent->inod;

    if (!S_ISDIR(inod->mode))
        return -ENOTDIR;

    tmp = current->cwd;
    current->cwd = dent;
    dput(tmp);
    return 0;
}
