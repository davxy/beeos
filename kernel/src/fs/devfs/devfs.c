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

#include "devfs.h"
#include "dev.h"
#include "driver/tty.h"
#include "driver/ramdisk.h"
#include "driver/random.h"
#include "kmalloc.h"
#include "kprintf.h"
#include "list.h"
#include <errno.h>
#include <string.h>



struct devfs_inode {
    struct inode        base;
    struct list_link    link;
};

static struct list_link devfs_nodes;

static struct super_block devfs_sb;

static ino_t devfs_ino = 0;


static ssize_t devfs_inode_read(struct inode *inod, void *buf,
                                size_t count, size_t off)
{
    ssize_t n;

    switch (inod->rdev) {
    case DEV_TTY:
    case DEV_TTY0:
    case DEV_TTY1:
    case DEV_TTY2:
    case DEV_TTY3:
    case DEV_TTY4 :
    case DEV_CONSOLE:
        n = tty_read(inod->rdev, buf, count);
        break;
    case DEV_ZERO:
        memset(buf, 0, count);
        n = (ssize_t)count;
        break;
    case DEV_NULL:
        n = (ssize_t)count;
        break;
    case DEV_INITRD:
        n = ramdisk_read(buf, count, off);
        break;
    case DEV_MEM:
        n = -1;
        break;
    case DEV_KMEM:
        n = -1;
        break;
    case DEV_RANDOM:
    case DEV_URANDOM:
        n = random_read(buf, count);
        break;
    default:
        n = -ENODEV;
        break;
    }
    return n;
}


static ssize_t devfs_inode_write(struct inode *inod, const void *buf,
                                 size_t count, size_t off)
{
    ssize_t n;

    switch (inod->rdev) {
    case DEV_TTY:
    case DEV_TTY0:
    case DEV_TTY1:
    case DEV_TTY2:
    case DEV_TTY3:
    case DEV_TTY4 :
    case DEV_CONSOLE:
        n = tty_write(inod->rdev, buf, count);
        break;
    case DEV_ZERO:
    case DEV_NULL:
        n = (ssize_t)count;
        break;
    case DEV_INITRD:
        n = ramdisk_write(buf, count, off);
        break;
    case DEV_MEM:
        n = -1;
        break;
    case DEV_KMEM:
        n = -1;
        break;
    case DEV_RANDOM:
    case DEV_URANDOM:
        n = -1;
        break;
    default:
        n = -ENODEV;
        break;
    }
    return n;
}


#define NDEVS 13

static struct {
    const char *name;
    dev_t       dev;
} dev_name_map[NDEVS] = {
    { "zero",    DEV_ZERO },
    { "tty0",    DEV_TTY0 },
    { "tty1",    DEV_TTY1 },
    { "tty2",    DEV_TTY2 },
    { "tty3",    DEV_TTY3 },
    { "tty4",    DEV_TTY4 },
    { "tty",     DEV_TTY  },
    { "console", DEV_CONSOLE },
    { "initrd",  DEV_INITRD },
    { "mem",     DEV_MEM },
    { "kmem",    DEV_KMEM },
    { "random",  DEV_RANDOM },
    { "urandom", DEV_URANDOM },
};

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

static int devfs_mknod(struct inode *idir, mode_t mode, dev_t dev)
{
    struct inode *inod;
    int res = -1;

    inod = inode_create(idir->sb, ++devfs_ino, mode, idir->ops);
    if (inod != NULL) {
        if (S_ISBLK(mode) || S_ISCHR(mode))
            inod->rdev = dev;
        res = 0;
    }
    return res;
}

static struct inode *devfs_lookup(struct inode *dir, const char *name)
{
    struct devfs_inode *curr;
    struct inode *inod = NULL;
    struct list_link *curr_link = devfs_nodes.next;
    dev_t dev;

#if 0
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
        return dir;
#endif

    dev = name_to_dev(name);
    while (curr_link != &devfs_nodes) {
        curr = list_container(curr_link, struct devfs_inode, link);
        if (curr->base.rdev == dev) {
            inod = &curr->base;
            break;
        }
        curr_link = curr_link->next;
    }
    return inod;
}


static const struct inode_ops devfs_inode_ops = {
    .read    = devfs_inode_read,
    .write   = devfs_inode_write,
    .mknod   = devfs_mknod,
    .lookup  = devfs_lookup,
};



static struct devfs_inode *devfs_sb_inode_alloc(struct super_block *sb)
{
    struct devfs_inode *inod;

    inod = (struct devfs_inode *)kmalloc(sizeof(struct devfs_inode), 0);
    if (inod == NULL)
        return NULL;
    list_init(&inod->link);
    list_insert_before(&devfs_nodes, &inod->link);
    return inod;
}

static void devfs_sb_inode_free(struct devfs_inode *inod)
{
    list_delete(&inod->link);
    kfree(inod, sizeof(struct devfs_inode));
}

static const struct super_ops devfs_sb_ops = {
    .inode_alloc = (super_inode_alloc_t) devfs_sb_inode_alloc,
    .inode_free  = (super_inode_free_t)  devfs_sb_inode_free,
};



static struct inode *dev_to_inode(dev_t dev)
{
    struct devfs_inode *inod = NULL;
    struct list_link *curr = devfs_nodes.next;

    while (curr != &devfs_nodes) {
        inod = list_container(curr, struct devfs_inode, link);
        if (inod->base.rdev == dev)
            break;
        curr = curr->next;
    }
    return (struct inode *)inod;
}

ssize_t devfs_read(dev_t dev, void *buf, size_t size, size_t off)
{
    ssize_t n = -1;
    struct inode *inod;

    inod = dev_to_inode(dev);
    if (inod != NULL)
        n = devfs_inode_read(inod, buf, size, off);
    return n;
}

ssize_t devfs_write(dev_t dev, const void *buf, size_t size, size_t off)
{
    ssize_t n = -1;
    struct inode *inod;

    inod = dev_to_inode(dev);
    if (inod != NULL)
        n = devfs_inode_write(inod, buf, size, off);
    return n;
}


static int devfs_dentry_readdir(struct dentry *dir, unsigned int i,
                                struct dirent *dent)
{
    static const struct list_link *rd_curr_link;
    int res = -1;
    const char *name = NULL;
    const struct dentry *curr;

    if (i == 0) {
        name = ".";
    } else if (i == 1) {
        name = "..";
    } else {
        if (i == 2)
            rd_curr_link = dir->child.next;
        if (rd_curr_link != &dir->child) {
            curr = list_container(rd_curr_link, struct dentry, link);
            name = curr->name;
            rd_curr_link = rd_curr_link->next;
            dent->d_ino = curr->inod->ino;
        }
    }
    if (name != NULL) {
        strncpy(dent->d_name, name, sizeof(dent->d_name));
        res = 0;
    }
    return res;
}

static const struct dentry_ops devfs_dentry_ops = {
    .readdir = devfs_dentry_readdir,
};

struct super_block *devfs_sb_get(void)
{
    return &devfs_sb;
}


struct super_block *devfs_super_create(dev_t dev)
{
    struct inode *iroot;
    struct dentry *droot;
    struct super_block *sb = NULL;

    /* First call */
    if (devfs_nodes.next == NULL)
        list_init(&devfs_nodes);

    droot = dentry_create("/", NULL, &devfs_dentry_ops);
    if (droot != NULL) {
        super_init(&devfs_sb, dev, droot, &devfs_sb_ops);

        iroot = inode_create(&devfs_sb, ++devfs_ino, S_IFDIR, &devfs_inode_ops);
        if (iroot != NULL) {
            droot->inod = idup(iroot);
            list_init(&devfs_nodes);
            sb = &devfs_sb;
        }
    }

    return sb;
}
