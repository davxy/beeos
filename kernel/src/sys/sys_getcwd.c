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

char *sys_getcwd(char *buf, size_t size)
{
    int i, j, status = 0;
    size_t slen;
    struct dirent dent;
    struct inode *icurr, *iparent;
    if (buf == NULL)
        return (char *)-EINVAL;

    icurr = current_task->cwd;
    
    j = 0;
    while (status == 0)
    {
        if (j+1 >= size)
            return (char *)-ENAMETOOLONG;
        buf[j++] = '/';
        iparent = fs_lookup(icurr, "..");
        if (iparent == icurr)
            break;
        i = 0;
        while ((status = fs_readdir(iparent, i++, &dent)) == 0)
        {
            if (dent.d_ino == icurr->ino)
            {
                slen = strlen(dent.d_name);
                if (j + slen >= size)
                    return (char *)-ENAMETOOLONG;
                memcpy(&buf[j], dent.d_name, slen);
                j += slen;
                icurr = iparent;
                break;
            }
        }
    }
    buf[j] = '\0';

    return buf;
}
