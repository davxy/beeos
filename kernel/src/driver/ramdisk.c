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

#include "driver/ramdisk.h"
#include <sys/types.h>
#include <string.h>

struct ramdisk
{
    void  *addr;
    size_t size;
};

struct ramdisk ramdisk;

void ramdisk_init(void *addr, size_t size)
{
    ramdisk.addr = addr;
    ramdisk.size = size;
}

static ssize_t ramdisk_rw_block(void *buf, size_t blocknum, int doread)
{
    ssize_t n = BLOCK_SIZE;
    size_t off = blocknum * BLOCK_SIZE;

    if (off > ramdisk.size)
        return -1;
    if (off + n > ramdisk.size)
        n = ramdisk.size - off;
    
    if (doread)
        memcpy(buf, (char *)ramdisk.addr + off, n);
    else
        memcpy((char *)ramdisk.addr + off, buf, n);
    return n;
}

ssize_t ramdisk_read_block(void *buf, size_t blocknum)
{
    return ramdisk_rw_block(buf, blocknum, 1);
}

ssize_t ramdisk_write_block(void *buf, size_t blocknum)
{
    return ramdisk_rw_block(buf, blocknum, 0);
}
