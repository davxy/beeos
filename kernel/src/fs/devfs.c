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

#include "devfs.h"
#include "dev.h"
#include "driver/tty.h"
#include "driver/ramdisk.h"
#include "driver/random.h"
#include "kmalloc.h"
#include "kprintf.h"
#include "list.h"
#include <errno.h>



struct devfs_inode {
    struct inode        base;
    struct list_link    link;
};

static struct list_link devfs_nodes;

static struct super_block devfs_sb;

static int devfs_ino = 0;


static ssize_t devfs_inode_read(struct inode *inode, void *buf,
                                size_t count, off_t offset)
{
    ssize_t n;
    dev_t rdev = inode->rdev;

    switch (rdev)
    {
        case DEV_TTY:
        case DEV_CONSOLE:
        case DEV_CONSOLE1:
        case DEV_CONSOLE2:
        case DEV_CONSOLE3:
        case DEV_CONSOLE4 :
            n = tty_read(rdev, buf, count);
            break;
        case DEV_ZERO:
            memset(buf, 0, count);
            n = count;
            break;
        case DEV_NULL:
            n = count;
            break;
        case DEV_INITRD:
            n = ramdisk_read(buf, count, offset);
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


static ssize_t devfs_inode_write(struct inode *inode, const void *buf,
                                 size_t count, off_t offset)
{
    ssize_t n;
    dev_t rdev = inode->rdev;

    switch (rdev)
    {
        case DEV_TTY:
        case DEV_CONSOLE:
        case DEV_CONSOLE1:
        case DEV_CONSOLE2:
        case DEV_CONSOLE3:
        case DEV_CONSOLE4 :
            n = tty_write(rdev, buf, count);
            break;
        case DEV_ZERO:
        case DEV_NULL:
            n = count;
            break;
        case DEV_INITRD:
            n = ramdisk_write(buf, count, offset);
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


static struct {
    const char *name;
    dev_t    dev;
} dev_name_map[] = {
    { "zero", DEV_ZERO },
    { "tty1", DEV_CONSOLE1 },
    { "tty2", DEV_CONSOLE2 },
    { "tty3", DEV_CONSOLE3 },
    { "tty4", DEV_CONSOLE4 },
    { "initrd", DEV_INITRD },
    { "mem", DEV_MEM },
    { "kmem", DEV_KMEM },
    { "random", DEV_RANDOM },
    { "urandom", DEV_URANDOM },
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

static int devfs_mknod(struct inode *idir, mode_t mode, dev_t dev)
{
    struct inode *inode;
    int res = -1;

    inode = idir->sb->ops->inode_alloc(idir->sb);
    if (inode != NULL)
    {
        inode_init(inode, idir->sb, ++devfs_ino, mode, dev, idir->ops);
        res = 0;
    }

    return res;
}

static struct inode *devfs_lookup(struct inode *dir, const char *name)
{
    struct devfs_inode *curr;
    struct inode *inode = NULL;
    struct list_link *curr_link = devfs_nodes.next;
    dev_t dev;

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
        return dir;

    dev = name_to_dev(name);
    while (curr_link != &devfs_nodes)
    {
        curr = list_container(curr_link, struct devfs_inode, link);
        if (curr->base.rdev == dev) {
            inode = &curr->base;
            break;
        }
        curr_link = curr_link->next;
    }
    return inode;
}

static const struct inode_ops devfs_inode_ops = {
    .read    = devfs_inode_read,
    .write   = devfs_inode_write,
    .mknod   = devfs_mknod,
    .lookup  = devfs_lookup,
};



static struct devfs_inode *devfs_sb_inode_alloc(struct super_block *sb)
{
    struct devfs_inode *inode;

    inode = kmalloc(sizeof(struct devfs_inode), 0);
    if (inode == NULL)
        return NULL;

    list_insert_before(&devfs_nodes, &inode->link);
    return inode;
}

static void devfs_sb_inode_free(struct devfs_inode *inode)
{
    list_delete(&inode->link);
    kfree(inode, sizeof(struct devfs_inode));
}

static const struct super_ops devfs_sb_ops = {
    .inode_alloc = (super_inode_alloc_t) devfs_sb_inode_alloc,
    .inode_free  = (super_inode_free_t)  devfs_sb_inode_free,
};



static struct inode *dev_to_inode(dev_t dev)
{
    struct devfs_inode *inode;
    struct list_link *curr = devfs_nodes.next;

    while (curr != &devfs_nodes)
    {
        inode = list_container(curr, struct devfs_inode, link);
        if (inode->base.rdev == dev)
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


static int devfs_dentry_readdir(struct dentry *dir, unsigned int i,
                                struct dirent *dent)
{
    static struct list_link *curr_link;
    int res = -1;
    const char *name = NULL;
    const struct dentry *curr;

    if (i == 0) {
        name = ".";
    }
    else if (i == 1)
        name = "..";
    else {
        if (i == 2)
            curr_link = dir->child.next;
        if (curr_link != &dir->child) {
            curr = list_container(curr_link, struct dentry, link);
            name = curr->name;
            curr_link = curr_link->next;
            dent->d_ino = curr->inod->ino;
        }
    }
    if (name) {
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
    struct devfs_inode *iroot;
    struct dentry *droot;

    list_init(&devfs_nodes);

    droot = dentry_create("/", NULL, &devfs_dentry_ops);

    super_init(&devfs_sb, dev, droot, &devfs_sb_ops);

    iroot = devfs_sb_inode_alloc(&devfs_sb);
    inode_init(&iroot->base, &devfs_sb, ++devfs_ino, S_IFDIR, 0,
               &devfs_inode_ops);

    droot->inod = &iroot->base;
    iget(droot->inod);

    return &devfs_sb;
}
