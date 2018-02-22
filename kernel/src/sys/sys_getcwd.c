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
#include <errno.h>
#include <string.h>


extern struct dentry *follow_up(struct dentry *root);


char *sys_getcwd(char *buf, size_t size)
{
    int j;
    size_t slen;
    struct dentry *curr;

    if (buf == NULL)
        return (char *)-EINVAL;

    curr = current_task->cwd;

    j = size;
    while (1) {
        if (strcmp(curr->name, "/") == 0)
            curr = follow_up(curr);
        if (curr == curr->parent)
            break;

        slen = strlen(curr->name);
        if (j - slen < 0)
            return (char *)-ENAMETOOLONG;
        j -= slen;
        memcpy(&buf[j], curr->name, slen);
        if (j - 1 < 0)
            return (char *)-ENAMETOOLONG;
        buf[--j] = '/';

        curr = curr->parent;
    }
    if (j == size)
        buf[--j] = '/';

    size -= j;
    memmove(buf, buf + j, size);
    buf[size] = '\0';

    return buf;
}
