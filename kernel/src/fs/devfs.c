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


#include "vfs.h"
#include "dev.h"
#include "driver/tty.h"
#include "driver/ramdisk.h"
#include "kmalloc.h"
#include "kprintf.h"
#include <errno.h>


//#define DEBUG_DEVFS


static struct sb devfs_sb;

struct devfs_inode *devfs_nodes;

struct devfs_inode {
    struct inode        base;
    struct devfs_inode  *next;
};


static ssize_t devfs_inode_read(struct inode *inode, void *buf,
                                size_t count, off_t offset)
{
	ssize_t n;
	dev_t dev = inode->dev;

#ifdef DEBUG_DEVFS
    kprintf(">>> devfs-read (dev=%04x)\n", inode->dev);
#endif

    switch (dev)
    {
        case DEV_TTY:
        case DEV_CONSOLE:
        case DEV_CONSOLE1:
        case DEV_CONSOLE2:
        case DEV_CONSOLE3:
        case DEV_CONSOLE4 :
            n = tty_read(dev, buf, count);
            break;
        case DEV_INITRD:
            n = ramdisk_read(buf, count, offset);
            break;
        case DEV_ZERO:
            memset(buf, 0, count);
            /* no break */
        case DEV_NULL:
            n = count;
            break;
        default:
            return -ENODEV;
    }
    return n;
}


static ssize_t devfs_inode_write(struct inode *inode, const void *buf,
                                 size_t count, off_t offset)
{
	ssize_t n;
	dev_t dev = inode->dev;

#ifdef DEBUG_DEVFS
    kprintf(">>> devfs-write (dev=%04x)\n", inode->dev);
#endif

    switch (dev)
    {
        case DEV_TTY:
        case DEV_CONSOLE:
        case DEV_CONSOLE1:
        case DEV_CONSOLE2:
        case DEV_CONSOLE3:
        case DEV_CONSOLE4 :
            n = tty_write(dev, buf, count);
            break;
        case DEV_INITRD:
            n = ramdisk_write(buf, count, offset);
            break;
        case DEV_ZERO:
        case DEV_NULL:
            n = count;
            break;
        default:
            return -ENODEV;
    }
    return n;
}


struct {
    const char *name;
    dev_t    dev;
} dev_name_map[] = {
    { "zero", DEV_ZERO },
    { "tty1", DEV_CONSOLE1 },
    { "tty2", DEV_CONSOLE2 },
    { "tty3", DEV_CONSOLE3 },
    { "tty4", DEV_CONSOLE4 },
};

#define NDEVS (sizeof(dev_name_map)/sizeof(dev_name_map[0]))

static dev_t name_to_dev(const char *name)
{
    int i;
    dev_t dev = 0xFFFF;

    for (i = 0; i < NDEVS; i++) {
        if (strcmp(dev_name_map[i].name, name) == 0) {
            dev = dev_name_map[i].dev;
            break;
        }
    }
    return dev;
}


struct inode *devfs_lookup(struct inode *dir, const char *name)
{
    struct devfs_inode *curr = devfs_nodes;
    dev_t dev;

#ifdef DEBUG_DEVFS
    kprintf(">>> devfs-lookup: %s\n", name);
#endif

    if (strcmp(name, ".") == 0)
        return dir;
    if (strcmp(name, "..") == 0)
        return NULL; /* TODO URGENT */

    dev = name_to_dev(name);
    while (curr != NULL)
    {
        if (curr->base.dev == dev)
            break;
        curr = curr->next;
    }
    return &curr->base;
}


static int devfs_readdir(struct inode *inode, unsigned int i,
                         struct dirent *dent)
{
    int res = -1;
    static struct devfs_inode *curr;
    const char *name = NULL;

#ifdef DEBUG_DEVFS
    kprintf(">>> devfs-readdir\n");
#endif

    if (i == 0)
        curr = devfs_nodes;

    while (curr != NULL && name == NULL)
    {
        switch (curr->base.dev)
        {
        case DEV_ZERO:
            name = "zero";
            break;
        case DEV_CONSOLE1:
            name = "tty1";
            break;
        case DEV_CONSOLE2:
            name = "tty2";
            break;
        case DEV_CONSOLE3:
            name = "tty3";
            break;
        case DEV_CONSOLE4:
            name = "tty4";
            break;
        default:
            curr = curr->next;
            break;
        }
    }

    if (name) {
        strncpy(dent->d_name, name, sizeof(dent->d_name));
        dent->d_ino = curr->base.ino;
        curr = curr->next;
        res = 0;
    }
    return res;
}

static const struct inode_ops devfs_inode_ops =
{
    .read    = devfs_inode_read,
    .write   = devfs_inode_write,
    .readdir = devfs_readdir,
    .lookup  = devfs_lookup,
};


static struct inode *devfs_sb_inode_alloc(void)
{
    struct devfs_inode *dinode;
    static int ino = 0;

    dinode = kmalloc(sizeof(struct devfs_inode), 0);
    if (!dinode)
        return NULL;
    inode_init(&dinode->base, 0, ino++);
    dinode->base.ops = &devfs_inode_ops;
    dinode->base.sb = &devfs_sb;
    dinode->next = devfs_nodes;
    devfs_nodes = dinode;
    return &dinode->base;
}

const struct sb_ops devfs_sb_ops =
{
    .inode_alloc = devfs_sb_inode_alloc,
};

void devfs_init(void)
{
    struct inode *root;

    devfs_nodes = NULL;

    root = fs_namei("/dev");
    if (root == NULL)
    {
        kprintf("Unable to mount devfs\nmount point \"/dev\" does not exits\n");
        return;
    }
    sb_init(&devfs_sb, 0, root, NULL);
    devfs_sb.ops = &devfs_sb_ops;
    root->sb = &devfs_sb;   /* This should be a sort of mount.... FIXME */
    root->ops = &devfs_inode_ops;
}

/*
 * Dinosaurus...
 *
 * PLEASE!!! KILL ME!!!
 */
ssize_t dev_io(dev_t dev, int rw, off_t off, void *buf, size_t size)
{
	ssize_t n = 0;
    struct devfs_inode *inode = devfs_nodes;

    if (dev == DEV_INITRD) /* FIXME: Temporary hack */
    {
        if (rw == DEV_READ)
            n = ramdisk_read(buf, size, off);
         else
            n = ramdisk_write(buf, size, off);
    }

    while (inode != NULL)
    {
        if (inode->base.dev == dev)
            break;
        inode = inode->next;
    }
    if (inode != NULL)
    {
        if (rw == DEV_READ)
            n = devfs_inode_read(&inode->base, buf, size, off);
        else
            n = devfs_inode_write(&inode->base, buf, size, off);
    }
    return n;
}
