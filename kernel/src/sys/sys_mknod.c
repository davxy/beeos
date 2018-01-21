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
#include "kprintf.h"
#include <limits.h>
#include <errno.h>
#include <string.h>


static void get_parent_dir(char *parent, const char *filepath)
{
    int i;

    i = strlen(filepath);
    while (i > 0) {
        i--;
        if (filepath[i] == '/')
            break;
    }
    strncpy(parent, filepath, i);
    parent[i] = '\0';
}

int sys_mknod(const char *pathname, mode_t mode, dev_t dev)
{
    struct inode *inode, *idir;
    char parent[PATH_MAX];

    inode = fs_namei(pathname);
    if (inode != NULL)
    {
        iput(inode);
        return -EEXIST; /* file exists */
    }

    get_parent_dir(parent, pathname);
    idir = fs_namei(parent);
    if (idir == NULL)
        return -1;

    inode = idir->sb->ops->inode_alloc();
    inode->mode = mode;
    inode->dev = dev;

    iput(idir);

    return 0;
}
