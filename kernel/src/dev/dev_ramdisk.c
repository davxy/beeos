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

#include "dev.h"
#include "util.h"
#include "driver/ramdisk.h"
#include <string.h>

char blk[BLOCK_SIZE];

ssize_t dev_io_ramdisk(pid_t pid, dev_t dev, int rw, off_t off, 
        void *buf, size_t size, int *eof)
{
    unsigned int nblk;
    ssize_t left;
    size_t ioff;
    ssize_t n;
    char *ptr;

    left = size;
    nblk = off / BLOCK_SIZE;
    ioff = off % BLOCK_SIZE;
    ptr = (char *)buf;

    if (rw == DEV_READ)
    {
        if ((n = ramdisk_read_block(blk, nblk)) != BLOCK_SIZE)
            return n;
        n = MIN(BLOCK_SIZE-ioff, left);
        memcpy(ptr, &blk[ioff], n);
        left -= n;
        ptr += n;
        nblk++;
        while (left > 0)
        {
            if ((n = ramdisk_read_block(blk, nblk)) != BLOCK_SIZE)
                break;
            n = MIN(BLOCK_SIZE, left);
            memcpy(ptr, blk, n);
            left -= n;
            ptr += n;
            nblk++;
        }
    }
    else
    {
        /* TODO */
    }
    return size-left;
}
