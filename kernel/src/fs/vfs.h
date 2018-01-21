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

/**
 * http://www.tldp.org/LDP/tlk/ds/ds.html
 */

#ifndef _BEEOS_FS_H_
#define _BEEOS_FS_H_

#include <htable.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

struct sb_ops
{
    struct inode *(*inode_alloc)(void);
    void (*inode_free)(struct inode *inode);
    int (*inode_read)(struct inode *inode);
    int (*inode_write)(struct inode *inode);
};

/** File system super block */
struct sb
{
    dev_t dev;              /** Device */
    struct inode *root;     /** Root inode */
    const struct sb_ops *ops;     /** Superblock operations */
};

void sb_init(struct sb *sb, dev_t dev, struct inode *root,
        const struct sb_ops *ops);

struct sb *vfs_sb_create(dev_t dev, const char *name);

/** Initial file system descriptor */
struct fs_type
{
    const char *name;
    struct sb *(*sb_create)(dev_t dev);
};

struct inode;

typedef int (*inode_read_t)(struct inode *inode, void *buf, 
            size_t count, off_t offset);

typedef int (*inode_write_t)(struct inode *inode, const void *buf,
             size_t count, off_t offset);


struct inode_ops
{
    inode_read_t  read;
    inode_write_t write;
    struct inode *(*lookup)(struct inode *dir, const char *name);
    int (*readdir)(struct inode *inode, unsigned int i,
            struct dirent *dent);
};

/** In-memory inode. */
struct inode
{
    dev_t       dev;    /* Device */
    dev_t       rdev;   /* Real device */
    mode_t      mode;   /* File type and permissions */
    uid_t       uid;    /* Ownner */
    gid_t       gid;    /* Group */
    ino_t       ino;    /* Inode number */
    size_t      size;   /* File size in bytes. */
    int         ref;    /* Reference count. */
    struct htable_link      hlink;
    struct sb   *sb;    /* Inode superblock */
    const struct inode_ops  *ops; /* VFS operations. */
};


struct file
{
    int flags;           /**< File status flags and file access modes. */
    int refs;            /**< Number of references. */
    off_t offset;        /**< File position. */
    struct inode *inode; /**< Inode reference. */
};

struct fd
{
    int flags;          /**< File descriptor flags, currently FD_CLOEXEC. */
    struct file *file;  /**< Pointer to the file table. */
};


static inline struct inode *fs_lookup(struct inode *dir, const char *name)
{
    struct inode *iret = 0;
    if (S_ISDIR(dir->mode) && dir->ops->lookup)
        iret = dir->ops->lookup(dir, name);
    return iret;
}

static inline int fs_readdir(struct inode *dir, int i, struct dirent *dent)
{
    int ret = -1;
    if (S_ISDIR(dir->mode) && dir->ops->readdir)
        ret = dir->ops->readdir(dir, i, dent);
    return ret;
}

static inline ssize_t fs_read(struct inode *node, void *buf,
        size_t count, off_t offset)
{
    int ret = -1;
    if (!S_ISDIR(node->mode) && node->ops->read)
        ret = node->ops->read(node, buf, count, offset);
    return ret;
}

static inline ssize_t fs_write(struct inode *node, const void *buf,
        size_t count, off_t offset)
{
    int ret = -1;
    if (!S_ISDIR(node->mode) && node->ops->write)
        ret = node->ops->write(node, buf, count, offset);
    return ret;
}

int fs_init(void);


struct inode *fs_namei(const char *pathname);

struct inode *iget(dev_t dev, ino_t ino);
struct inode *idup(struct inode *inode);
void iput(struct inode *inode);

struct inode *inode_lookup(dev_t dev, ino_t ino);
void inode_init(struct inode *inode, dev_t dev, ino_t ino);
struct inode *inode_create(dev_t dev, ino_t ino);

#endif /* _BEEOS_FS_H_ */
