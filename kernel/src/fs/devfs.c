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
#include "list.h"
#include <errno.h>


//#define DEBUG_DEVFS


static struct sb devfs_sb;

static struct list_link devfs_nodes;

struct devfs_inode {
    struct inode        base;
    struct list_link    link;
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
    { "initrd", DEV_INITRD },
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

static const char *dev_to_name(dev_t dev)
{
    int i;
    const char *name = "?";

    for (i = 0; i < NDEVS; i++) {
        if (dev_name_map[i].dev == dev) {
            name = dev_name_map[i].name;
            break;
        }
    }
    return name;
}


struct inode *devfs_lookup(struct inode *dir, const char *name)
{
    struct devfs_inode *curr;
    struct inode *inode = NULL;
    struct list_link *curr_link = devfs_nodes.next;
    dev_t dev;

#ifdef DEBUG_DEVFS
    kprintf(">>> devfs-lookup: %s\n", name);
#endif

    if (strcmp(name, ".") == 0)
        return dir;
    if (strcmp(name, "..") == 0)
        return dir->sb->mnt->root;

    dev = name_to_dev(name);
    while (curr_link != &devfs_nodes)
    {
    	curr = list_container(curr_link, struct devfs_inode, link);
        if (curr->base.dev == dev) {
        	inode = &curr->base;
        	break;
        }
        curr_link = curr_link->next;
    }
    return inode;
}


static int devfs_readdir(struct inode *inode, unsigned int i,
                         struct dirent *dent)
{
	static struct list_link *curr_link;
    int res = -1;
    struct devfs_inode *curr;
    const char *name = NULL;

#ifdef DEBUG_DEVFS
    kprintf(">>> devfs-readdir\n");
#endif

    if (i == 0)
        name = ".";
    else if (i == 1)
        name = "..";
    else {
    	if (i == 2)
    		curr_link = devfs_nodes.next;
    	if (curr_link != &devfs_nodes) {
    		curr = list_container(curr_link, struct devfs_inode, link);
            name = dev_to_name(curr->base.dev);
            curr_link = curr_link->next;
            dent->d_ino = curr->base.ino;
    	}
    }

    if (name) {
        strncpy(dent->d_name, name, sizeof(dent->d_name));
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
    struct devfs_inode *inode;
    static int ino = 0;

    inode = kmalloc(sizeof(struct devfs_inode), 0);
    if (inode == NULL)
        return NULL;
    inode_init(&inode->base, 0, ino++);
    inode->base.ops = &devfs_inode_ops;
    inode->base.sb = &devfs_sb;
    list_insert_before(&devfs_nodes, &inode->link);
    return &inode->base;
}

const struct sb_ops devfs_sb_ops =
{
    .inode_alloc = devfs_sb_inode_alloc,
};

struct sb *devfs_sb_get(void)
{
	return &devfs_sb;
}

struct sb *devfs_init(void)
{
    struct inode *root;

    list_init(&devfs_nodes);

    root = devfs_sb_inode_alloc();
    root->sb = &devfs_sb;
    root->ops = &devfs_inode_ops;

    sb_init(&devfs_sb, 0, root, NULL);
    devfs_sb.ops = &devfs_sb_ops;

    return &devfs_sb;
}

static struct inode *dev_to_inode(dev_t dev)
{
    struct devfs_inode *inode;
    struct list_link *curr = devfs_nodes.next;

    while (curr != &devfs_nodes)
    {
    	inode = list_container(curr, struct devfs_inode, link);
        if (inode->base.dev == dev)
            break;
        curr = curr->next;
    }
    return (struct inode *)inode;
}

ssize_t devfs_read(dev_t dev, void *buf, size_t size, off_t off)
{
    ssize_t n = -1;
    struct inode *inode;

    inode = dev_to_inode(dev);
    if (inode != NULL)
        n = devfs_inode_read(inode, buf, size, off);
    return n;
}

ssize_t devfs_write(dev_t dev, const void *buf, size_t size, off_t off)
{
    ssize_t n = -1;
    struct inode *inode;

    inode = dev_to_inode(dev);
    if (inode != NULL)
        n = devfs_inode_write(inode, buf, size, off);
    return n;
}

