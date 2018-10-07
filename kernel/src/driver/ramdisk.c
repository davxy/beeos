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

#include "driver/ramdisk.h"
#include "util.h"
#include <sys/types.h>
#include <string.h>


#define BLOCK_SIZE  512


static struct {
    void  *addr;
    size_t size;
} ramdisk;


static ssize_t ramdisk_rw_block(void *buf, size_t blocknum, int doread)
{
    size_t n = BLOCK_SIZE;
    size_t off;

    off = blocknum * BLOCK_SIZE;
    if (off > ramdisk.size)
        return -1;
    if (off + n > ramdisk.size)
        n = ramdisk.size - off;

    if (doread == 1)
        memcpy(buf, (char *)ramdisk.addr + off, n);
    else
        memcpy((char *)ramdisk.addr + off, buf, n);
    return (ssize_t)n;
}

static ssize_t ramdisk_read_block(void *buf, size_t blocknum)
{
    return ramdisk_rw_block(buf, blocknum, 1);
}

#if 0
static ssize_t ramdisk_write_block(void *buf, size_t blocknum)
{
    return ramdisk_rw_block(buf, blocknum, 0);
}
#endif

static char blk[BLOCK_SIZE];

ssize_t ramdisk_read(void *buf, size_t size, size_t off)
{
    unsigned int nblk;
    size_t left;
    size_t ioff;
    ssize_t n;
    char *ptr;

    left = size;
    nblk = off / BLOCK_SIZE;
    ioff = off % BLOCK_SIZE;
    ptr = (char *)buf;

    if ((n = ramdisk_read_block(blk, nblk)) != BLOCK_SIZE)
        return n;
    n = MIN(BLOCK_SIZE - ioff, left);
    memcpy(ptr, &blk[ioff], n);
    left -= n;
    ptr += n;
    nblk++;
    while (left > 0) {
        if ((n = ramdisk_read_block(blk, nblk)) != BLOCK_SIZE)
            break;
        n = MIN(BLOCK_SIZE, left);
        memcpy(ptr, blk, n);
        left -= n;
        ptr += n;
        nblk++;
    }

    return (ssize_t)(size - left);
}

ssize_t ramdisk_write(const void *buf, size_t size, size_t off)
{
    return -1; /* TODO */
}


void ramdisk_init(void *addr, size_t size)
{
    ramdisk.addr = addr;
    ramdisk.size = size;
}
