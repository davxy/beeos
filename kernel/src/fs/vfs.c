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

#include "fs/vfs.h"
#include "fs/devfs/devfs.h"   /* devfs_super_create */
#include "fs/ext2/ext2.h"    /* ext2_super_create */
#include "mm/slab.h"
#include "kmalloc.h"
#include "proc.h"
#include "panic.h"
#include <limits.h>
#include <errno.h>

#ifdef DEBUG_VFS
#include "kprintf.h"
#endif

#define FS_LIST_LEN 2

static const struct vfs_type fs_list[FS_LIST_LEN] = {
    { "ext2", ext2_super_create },
    { "dev",  devfs_super_create }
};


void super_init(struct super_block *sb, dev_t dev, struct dentry *root,
                const struct super_ops *ops)
{
    sb->dev = dev;
    sb->root = ddup(root);
    sb->ops = ops;
}

struct super_block *vfs_super_create(dev_t dev, const char *name)
{
    int i;
    struct super_block *sb = NULL;

    for (i = 0; i < FS_LIST_LEN; i++) {
        if (strcmp(name, fs_list[i].name) == 0) {
            sb = fs_list[i].create(dev);
            break;
        }
    }
    return sb;
}



static struct slab_cache inode_cache;
static struct slab_cache file_cache;

#define INODE_HTABLE_BITS     3   /* 8 elements hash table (TODO change) */
static struct htable_link *inode_htable[1 << INODE_HTABLE_BITS];

#define KEY(dev, ino)    (((dev) << 16) + (ino))


struct file *fs_file_alloc(void)
{
    return (struct file *)slab_cache_alloc(&file_cache, 0);
}

void fs_file_free(struct file *fil)
{
    slab_cache_free(&file_cache, fil);
}

static struct inode *inode_lookup(dev_t dev, ino_t ino)
{
    struct inode *ip;
    struct htable_link *lnk;

    lnk = htable_lookup(inode_htable, KEY(dev,ino), INODE_HTABLE_BITS);
    while (lnk != NULL) {
        ip = struct_ptr(lnk, struct inode, hlink);
        if (ip->ref > 0 && ip->sb->dev == dev && ip->ino == ino)
            return ip;
        lnk = lnk->next;
    }
    return NULL;
}


static void inode_init(struct inode *inod, struct super_block *sb,
                       ino_t ino, mode_t mode, const struct inode_ops *ops)
{
    memset(inod, 0, sizeof(*inod));

    inod->ops = ops;
    inod->ino = ino;
    inod->mode = mode;
    inod->sb  = sb;

    /*
     * TODO: consider the inode read return value.
     * This is just a quick and dirty solution for MISRA compliance
     */
    if (sb->ops->inode_read != NULL)
        sb->ops->inode_read(inod);

    htable_insert(inode_htable, &inod->hlink,
                  KEY(inod->sb->dev, inod->ino), INODE_HTABLE_BITS);
}


struct inode *inode_create(struct super_block *sb, ino_t ino, mode_t mode,
                           const struct inode_ops *ops)
{
    struct inode *inod;

    inod = sb->ops->inode_alloc(sb);
    if (inod != NULL)
        inode_init(inod, sb, ino, mode, ops);
    return inod;
}

void inode_delete(struct inode *inod)
{
    /* Check if was in the hash table (e.g. pipe inodes are not) */
    if (inod->hlink.pprev != NULL)
        htable_delete(&inod->hlink);

    if (inod->sb->ops->inode_free != NULL)
        inod->sb->ops->inode_free(inod);
    else
        slab_cache_free(&inode_cache, inod);
}



void iput(struct inode *inod)
{
    inod->ref--;
#ifdef DEBUG_VFS
    kprintf("iput: ino=%d, ref=%d\n", inod->ino, inod->ref);
    if (inod->ref < 0)
        kprintf("WARNING iref < 0\n");
#endif
    if (inod->ref == 0)
        inode_delete(inod);
}

struct inode *iget(struct super_block *sb, ino_t ino)
{
    struct inode *inod;

    inod = inode_lookup(sb->dev, ino);
    if (inod == NULL) {
        inod = inode_create(sb, ino, 0, sb->root->inod->ops);
        if (inod == NULL)
            return NULL;
    }
    inod->ref++;
#ifdef DEBUG_VFS
    kprintf("iget: ino=%d, ref=%d\n", inod->ino, inod->ref);
#endif
    return inod;
}


/*
 * Copy the next path element from path into name.
 * Return a pointer to the element following the copied one.
 * The returned path has no leading slashes,
 * so the caller can check *path=='\0' to see if the name is the last one.
 * If no name to remove, return 0.
 *
 * Examples:
 *   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
 *   skipelem("///a//bb", name) = "bb", setting name = "a"
 *   skipelem("a", name) = "", setting name = "a"
 *   skipelem("", name) = skipelem("////", name) = 0
 *
 */
static const char *skipelem(const char *path, char *name)
{
    const char *s;
    int len;

    while(*path == '/')
        path++;
    if(*path == 0)
        return NULL;
    s = path;
    while(*path != '/' && *path != 0)
        path++;
    len = path - s;
    if(len >= NAME_MAX) {
        memmove(name, s, NAME_MAX);
    } else {
        memmove(name, s, len);
        name[len] = 0;
    }
    while(*path == '/')
        path++;
    return path;
}



struct dentry *dentry_create(const char *name, struct dentry *parent,
                             const struct dentry_ops *ops)
{
    struct dentry *de;

    de = (struct dentry *)kmalloc(sizeof(*de), 0);
    if (de == NULL)
        return NULL;
    strcpy(de->name, name);
    de->ref = 0;
    de->inod = NULL; /* May be without an inode */
    de->parent = (parent != NULL) ? parent : de;
    list_init(&de->child);  /* Empty children list */
    list_insert_before(&de->parent->child, &de->link); /* Insert in the parent child  list */
    de->mounted = 0;
    de->ops = ops;
    return de;
}

void dentry_delete(struct dentry *dent)
{
    struct list_link *curr;
    struct dentry *curr_de;

    /* Eventually remove the reference in the children */
    curr = dent->child.next;
    while (curr != &dent->child) {
        curr_de = list_container(curr, struct dentry, link);
        curr_de->parent = NULL;
        curr = curr->next;
    }

    /* Delete from siblings list */
    list_delete(&dent->link);

    kfree(dent, sizeof(struct dentry));
}

static struct dentry *dentry_lookup(const struct dentry *dir, const char *name)
{
    struct list_link *curr;
    struct dentry *curr_dent;
    struct dentry *res = NULL;

    curr = dir->child.next;
    while (curr != &dir->child) {
        curr_dent = list_container(curr, struct dentry, link);
        if (strcmp(curr_dent->name, name) == 0) {
            res = curr_dent;
            break;
        }
        curr = curr->next;
    }
    return res;
}


struct dentry *dget(struct dentry *dir, const char *name)
{
    struct dentry *dent;
    struct inode  *inod;

    dent = dentry_lookup(dir, name);
    if (dent == NULL) {
        inod = vfs_lookup(dir->inod, name);
        if (inod == NULL)
            return NULL;
        dent = dentry_create(name, dir, dir->ops);
        if (dent == NULL)
            return NULL;
        dent->inod = idup(inod);
    }

    dent->ref++;
#ifdef DEBUG_VFS
    kprintf("dget: (%s) ino=%d, iref=%d, dref=%d\n",
            dent->name, dent->inod->ino, dent->inod->ref, dent->ref);
#endif
    return dent;
}

#if 0
/*
 * Actually this doesn't work...
 * For example doesn't handle the following case:
 * 1. I'm in the root dir
 * 2. I jump two levels. (e.g. cd /usr/bin)
 * The cwd is still '/' thus the 'usr' dentry reference is destroyed.
 * The getcwd thus is not able to reconstruct the path from the 'dentry parent'
 * hierarchy.
 */
static int dentry_can_delete(const struct dentry *dent)
{
    int res = 1;
    struct dentry *curr = current->cwd;

    while (curr->parent != curr) {
        if (curr == dent) {
            res = 0;
            break;
        }
        curr = curr->parent;
    }
    return res;
}
#endif

void dput(struct dentry *dent)
{
    dent->ref--;

#ifdef DEBUG_VFS
    kprintf("dput: (%s) ino=%d, iref=%d, dref=%d\n",
            dent->name, dent->inod->ino, dent->inod->ref, dent->ref);
    if (dent->ref < 0)
        kprintf("WARNING dref < 0\n");
#endif

#if 0
    if (dent->ref == 0 && dentry_can_delete(dent) != 0) {
        if (dent->inod != NULL)
            iput(dent->inod);
        dentry_delete(dent);
    }
#endif
}



static struct list_link mounts;

int do_mount(struct dentry *mntpt, struct dentry *root)
{
    struct vfsmount *mnt;

    mnt = (struct vfsmount *)kmalloc(sizeof(*mnt), 0);
    if (mnt == NULL)
        return -1;
    mnt->mntpt = ddup(mntpt);
    mnt->root  = ddup(root);
    list_insert_after(&mounts, &mnt->link);
    mntpt->mounted = 1;
    return 0;
}


static struct dentry *follow_up(struct dentry *root)
{
    struct dentry *res = root;
    const struct list_link *curr;
    const struct vfsmount *mnt;

    /* TODO: this is not efficient... use a hash map here */
    curr = mounts.next;
    while (curr != &mounts) {
        mnt = list_container(curr, struct vfsmount, link);
        if (mnt->root == res) {
            dput(mnt->mntpt);
            res = ddup(mnt->mntpt);
            /* Reiterate to see if this is a also mount point */
            curr = mounts.next;
        } else {
            curr = curr->next;
        }
    }
    return res;
}

static struct dentry *follow_down(struct dentry *mntpt)
{
    struct dentry *res = mntpt;
    const struct list_link *curr;
    const struct vfsmount *mnt;

    /* TODO: this is not efficient... use a hash map here */
    curr = mounts.next;
    while (curr != &mounts) {
        mnt = list_container(curr, struct vfsmount, link);
        if (mnt->mntpt == res) {
            dput(res);
            res = ddup(mnt->root);
            /* Reiterate to see if this is a also mount point */
            curr = mounts.next;
        } else {
            curr = curr->next;
        }
    }
    return res;
}



struct dentry *named(const char *path)
{
    char name[NAME_MAX];
    struct dentry *dent;
    struct dentry *tmp;

    if (path == NULL || *path == '\0')
        return NULL;

    dent = ddup((*path == '/') ? current->root : current->cwd);

    while ((path = skipelem(path, name)) != NULL) {
        if (!S_ISDIR(dent->inod->mode))
            return NULL;

        if (strcmp(name, ".") == 0) {
            continue;
        } else if (strcmp(name, "..") == 0) {
            if (strcmp(dent->name, "/") == 0)
                dent = follow_up(dent);
            tmp = ddup(dent->parent);
        } else {
            if (dent->mounted != 0)
                dent = follow_down(dent);
            tmp = dget(dent, name);
        }
        dput(dent);
        if (tmp == NULL)
            return NULL;
        dent = tmp;
    }
    if (S_ISDIR(dent->inod->mode) && dent->mounted != 0)
        dent = follow_down(dent);
    return dent;
}


int dentry_path(struct dentry *dent, char *buf, size_t size)
{
    int res = 0;
    size_t j;
    size_t slen;
    struct dentry *curr = dent;

    j = size;
    do {
        if (strcmp(curr->name, "/") == 0)
            curr = follow_up(curr);
        if (curr == curr->parent)
            break;

        slen = strlen(curr->name);
        if (slen < j) {
            j -= slen;
            memcpy(&buf[j], curr->name, slen);
            buf[--j] = '/';

            curr = curr->parent;
        } else {
            res = -ENAMETOOLONG;
        }
    } while (res == 0);

    if (res == 0) {
        if (j == size)
            buf[--j] = '/';
        size -= j;
        memmove(buf, buf + j, size);
        buf[size] = '\0';
    }
    return res;
}


void vfs_init(void)
{
    slab_cache_init(&inode_cache, "inode-cache", sizeof(struct inode),
            0, 0, NULL, NULL);

    slab_cache_init(&file_cache, "file-cache", sizeof(struct file),
            0, 0, NULL, NULL);

    htable_init(inode_htable, INODE_HTABLE_BITS);

    list_init(&mounts);
}
