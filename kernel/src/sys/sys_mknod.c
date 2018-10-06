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
#include "kprintf.h"
#include <limits.h>
#include <errno.h>
#include <string.h>


static void split_path(const char *filepath, char *parent, char *name)
{
    int i, k;

    i = strlen(filepath);
    while (i > 0) {
        i--;
        if (filepath[i] == '/')
            break;
    }
    if (i > 0) {
        k = (*filepath == '/') ? 1 : 0;
        strncpy(parent, filepath, i);
        strcpy(name, filepath + i + k);
    } else {
        parent[i++] = '/';
        if (filepath[i] != '\0') {
            strcpy(name, filepath + i);
        } else {
            name[0] = '.';
            name[1] = '\0';
        }
    }
    parent[i] = '\0';
}


int sys_mknod(const char *pathname, mode_t mode, dev_t dev)
{
    int res;
    struct dentry *dent;
    const struct dentry *dnew;
    char parent[PATH_MAX];
    char name[NAME_MAX];

    dent = named(pathname);
    if (dent != NULL) {
        dput(dent);
        return -EEXIST;
    }

    split_path(pathname, parent, name);

    dent = named(parent);
    if (dent == NULL)
        return -1;

    res = vfs_mknod(dent->inod, mode, dev);

    /*
     * Create a dentry and keep a reference to it.
     */
    if (res == 0) {
        dnew = dget(dent, name);
        if (dnew == NULL)
            res = -1;
    }
    dput(dent);
    return res;
}
