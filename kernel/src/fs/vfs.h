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

#ifndef BEEOS_FS_VFS_H_
#define BEEOS_FS_VFS_H_

#include "htable.h"
#include "list.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>


/*
 * Superblock declarations
 */


/** File system super block */
struct super_block
{
    dev_t                   dev;  /** Device */
    struct dentry          *root; /** Root dentry */
    const struct super_ops *ops;  /** Superblock vfs operations */
};

typedef struct inode * (* super_inode_alloc_t)(struct super_block *sb);
typedef void (* super_inode_free_t)(struct inode *inode);
typedef int (* super_inode_read_t)(struct inode *inode);
typedef int (* super_inode_write_t)(struct inode *inode);

struct super_ops {
    super_inode_alloc_t   inode_alloc;
    super_inode_free_t    inode_free;
    super_inode_read_t    inode_read;
    super_inode_write_t   inode_write;
};



void super_init(struct super_block *sb, dev_t dev, struct dentry *root,
                const struct super_ops *ops);

/**
 * Creates a file system super_block.
 */
struct super_block *vfs_super_create(dev_t dev, const char *name);

/** Initial file system descriptor */
struct vfs_type
{
    const char *name;
    struct super_block *(* create)(dev_t dev);
};



/*
 * Inode declarations.
 */

/** In-memory inode. */
struct inode {
    dev_t       rdev;   /**< Real device */
    mode_t      mode;   /**< File type and permissions */
    uid_t       uid;    /**< Owner user id */
    gid_t       gid;    /**< Owner group id */
    ino_t       ino;    /**< Inode number */
    size_t      size;   /**< File size in bytes. */
    int         ref;    /**< Reference counter. */
    time_t      atime;  /**< Access time */
    time_t      mtime;  /**< Modification time */
    time_t      ctime;  /**< Creation time */
    struct htable_link      hlink; /**< Link within the hash table */
    struct super_block      *sb;   /**< Inode superblock */
    const struct inode_ops  *ops;  /**< Inode vfs Operations */
};

typedef int (* inode_read_t)(struct inode *inode, void *buf,
            size_t count, off_t off);

typedef int (* inode_write_t)(struct inode *inode, const void *buf,
            size_t count, off_t off);

typedef int (* inode_mknod_t)(struct inode *idir, mode_t mode,
            dev_t dev);

typedef struct inode *(* inode_lookup_t)(struct inode *udir, const char *name);

struct inode_ops {
    inode_read_t    read;
    inode_write_t   write;
    inode_mknod_t   mknod;
    inode_lookup_t  lookup;
};


/*
 * Dentry declarations.
 */

struct dentry {
    char              name[NAME_MAX];  /**< Name */
    int               ref;             /**< Reference counter */
    struct inode     *inod;          /**< Inode */
    struct dentry    *parent;          /**< Parent directory */
    struct list_link  child;           /**< Children list (if is a dir) */
    struct list_link  link;            /**< Siblings link */
    int               mounted;         /**< Set to 1 if is a mount point */
    const struct dentry_ops *ops;      /**< Dentry vfs operations */
};

typedef int (* dentry_readdir_t)(struct dentry *dir, unsigned int i,
            struct dirent *dent);

struct dentry_ops {
    dentry_readdir_t readdir; /**< Read directory */
};



struct file {
    int            flags;   /**< File status flags and access modes. */
    int            ref;     /**< Reference counter. */
    mode_t         mode;    /**< File mode when a new file is created */
    off_t          off;     /**< File position. */
    struct dentry *dent;    /**< Dentry reference. */
};

struct fd {
    int          flags;  /**< File descriptor flags, currently FD_CLOEXEC. */
    struct file *fil;    /**< Pointer to the file table. */
};

struct vfsmount {
    struct dentry    *mntpt; /**< mount point */
    struct dentry    *root;  /**< mount root */
    struct list_link  link;  /**< link in the mounts list */
};



static inline struct inode *vfs_inode_alloc(struct super_block *sb)
{
    struct inode *inod = NULL;

    if (sb->ops->inode_alloc != NULL)
        inod = sb->ops->inode_alloc(sb);
    return inod;
}



static inline struct inode *vfs_lookup(struct inode *idir, const char *name)
{
    struct inode *iret = 0;

    if (S_ISDIR(idir->mode) && idir->ops->lookup)
        iret = idir->ops->lookup(idir, name);
    return iret;
}

static inline int vfs_mknod(struct inode *idir, mode_t mode, dev_t dev)
{
    int ret = -1;

    if (S_ISDIR(idir->mode) && idir->ops->mknod)
        ret = idir->ops->mknod(idir, mode, dev);
    return ret;
}

static inline ssize_t vfs_read(struct inode *node, void *buf,
        size_t count, off_t offset)
{
    int ret = -1;

    if (!S_ISDIR(node->mode) && node->ops->read)
        ret = node->ops->read(node, buf, count, offset);
    return ret;
}

static inline ssize_t vfs_write(struct inode *node, const void *buf,
        size_t count, off_t offset)
{
    int ret = -1;

    if (!S_ISDIR(node->mode) && node->ops->write)
        ret = node->ops->write(node, buf, count, offset);
    return ret;
}



static inline int vfs_readdir(struct dentry *dir, int i, struct dirent *dent)
{
    int ret = -1;

    if (S_ISDIR(dir->inod->mode) && dir->ops->readdir)
        ret = dir->ops->readdir(dir, i, dent);
    return ret;
}



struct inode *inode_lookup(dev_t dev, ino_t ino);

void inode_init(struct inode *inode, struct super_block *sb, ino_t ino,
                mode_t mode, dev_t dev, const struct inode_ops *ops);

struct inode *namei(const char *pathname);

void iget(struct inode *inode);

void iput(struct inode *inode);



struct dentry *dentry_create(const char *name, struct dentry *parent,
                             const struct dentry_ops *ops);

void dentry_delete(struct dentry *de);

struct dentry *named(const char *pathname);

void dget(struct dentry *de);

void dput(struct dentry *de);




int do_mount(struct dentry *mntpt, struct dentry *root);


int vfs_init(void);

#endif /* BEEOS_FS_VFS_H_ */

