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

#include "fs/vfs.h"
#include "mm/slab.h"
#include "kmalloc.h"
#include "proc.h"
#include "panic.h"


struct sb *ext2_sb_create(dev_t dev);
struct sb *devfs_sb_create(dev_t dev);

// http://www.tldp.org/LDP/lki/lki-3.html

struct fs_type fs_list[] =
{
    { "ext2", ext2_sb_create },
	{ "dev",  devfs_sb_create }
};

#define FS_LIST_LEN (sizeof(fs_list)/sizeof(fs_list[0]))

void sb_init(struct sb *sb, dev_t dev, struct dentry *root,
        const struct sb_ops *ops)
{
    sb->dev = dev;
    sb->root = root;
    sb->mnt = sb;
    sb->ops = ops;
}

struct sb *vfs_sb_create(dev_t dev, const char *type)
{
    int i;
    struct sb *sb;

    sb = NULL;
    for (i = 0; i < FS_LIST_LEN; i++)
    {
        if (strcmp(type, fs_list[i].name) == 0)
        {
            sb = fs_list[i].sb_create(dev);
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
    struct file *file = slab_cache_alloc(&file_cache, 0);
    return file;
}

void fs_file_free(struct file *file)
{
    slab_cache_free(&file_cache, file);
}

struct inode *fs_inode_alloc(void)
{
    return slab_cache_alloc(&inode_cache, 0);
}


struct inode *inode_lookup(dev_t dev, ino_t ino)
{
    struct inode *ip;
    struct htable_link *lnk;

    lnk = htable_lookup(inode_htable, KEY(dev,ino), INODE_HTABLE_BITS);
    while (lnk != NULL)
    {
        ip = struct_ptr(lnk, struct inode, hlink);
        if (ip->ref > 0 && ip->sb->dev == dev && ip->ino == ino)
        {
            ip->ref++;
            return ip;
        }
        lnk = lnk->next;
    }
    return NULL;
}


void inode_init(struct inode *inode, struct sb *sb, ino_t ino, mode_t mode,
                dev_t dev, const struct inode_ops *ops)
{
    memset(inode, 0, sizeof(*inode));

    inode->ops = ops;
    inode->ino = ino;
    inode->mode = mode;
    inode->sb  = sb;

    if (S_ISBLK(mode) || S_ISCHR(mode))
        inode->rdev = dev;

    if (sb->ops->inode_read != NULL)
        sb->ops->inode_read(inode);

    htable_insert(inode_htable, &inode->hlink,
                  KEY(inode->sb->dev, inode->ino), INODE_HTABLE_BITS);
}



#if 0
struct inode *inode_create(dev_t dev, ino_t ino)
{
    struct inode *inode;
    inode = fs_inode_alloc();
    if (inode == NULL)
        return NULL;
    inode_init(inode, dev, ino);
    return inode;
}

struct inode *iget(dev_t dev, ino_t ino)
{
    struct inode *inode;

    if ((inode = inode_lookup(dev, ino)) != NULL)
        return inode;

    inode = kmalloc(sizeof(*inode), 0);
    if (!inode)
        panic("iget: no free inodes");
    inode_init(inode, dev, ino);
    return inode;
}

void iput(struct inode *ip)
{
    ip->ref--;
    if (ip->ref != 0)
        return;
    /* Check if was in the hash table (e.g. pipe inodes are not) */
    if (ip->hlink.pprev != NULL)
    {
        /* TODO temporary code... 
         * If is a pipe the pipe_inode structure must be released differently */
        htable_delete(&ip->hlink); 
        slab_cache_free(&inode_cache, ip);
    }
}


struct inode *idup(struct inode *ip)
{
    ip->ref++;
    return ip;
}

#else

void iput(struct inode *ip)
{

}

struct inode *idup(struct inode *ip)
{
    return ip;
}

#endif


// Copy the next path element from path into name.
// Return a pointer to the element following the copied one.
// The returned path has no leading slashes,
// so the caller can check *path=='\0' to see if the name is the last one.
// If no name to remove, return 0.
//
// Examples:
//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
//
#define DIRSIZ  64

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
    if(len >= DIRSIZ)
        memmove(name, s, DIRSIZ);
    else
    {
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

    de = kmalloc(sizeof(*de), 0);
    if (!de)
        return NULL;
    strcpy(de->name, name);
    de->inode = NULL; /* May be without an inode */
    de->parent = (parent != NULL) ? parent : de;
    list_init(&de->child);  /* Empty children list */
    list_insert_before(&parent->child, &de->link); /* Insert in the parent child  list */
    de->mounted = 0;
    de->ops = ops;
    return de;
}

void dentry_destroy(struct dentry *de)
{
    list_delete(&de->link);
    kfree(de, sizeof(*de));
}

static struct list_link mounts;

int do_mount(struct dentry *mntpt, struct dentry *root)
{
    struct vfsmount *mnt;

    mnt = kmalloc(sizeof(*mnt), 0);
    if (mnt == NULL)
        return -1;
    mnt->mntpt = mntpt;
    mnt->root  = root;
    list_insert_after(&mounts, &mnt->link);
    mntpt->mounted = 1;
    return 0;
}


struct dentry *follow_up(struct dentry *root)
{
    struct dentry *res = root;
    struct vfsmount *mnt;
    struct list_link *curr;

    /* TODO: this is not efficient... use a hash map here */
    curr = mounts.next;
    while (curr != &mounts) {
        mnt = list_container(curr, struct vfsmount, link);
        if (mnt->root == res)
        {
            res = mnt->mntpt;
            /* Reiterate to see if this is a also mount point */
            curr = mounts.next;
        } else {
            curr = curr->next;
        }
    }
    return res;
}

struct dentry *follow_down(struct dentry *mntpt)
{
    struct dentry *res = mntpt;
    struct vfsmount *mnt;
    struct list_link *curr;

    /* TODO: this is not efficient... use a hash map here */
    curr = mounts.next;
    while (curr != &mounts) {
        mnt = list_container(curr, struct vfsmount, link);
        if (mnt->mntpt == res)
        {
            res = mnt->root;
            /* Reiterate to see if this is a also mount point */
            curr = mounts.next;
        } else {
            curr = curr->next;
        }
    }
    return res;
}


struct dentry *dentry_lookup(struct dentry *dir, const char *name)
{
    struct list_link *curr;
    struct dentry *curr_de, *res = NULL;

    curr = dir->child.next;
    while (curr != &dir->child) {
        curr_de = list_container(curr, struct dentry, link);
        if (strcmp(curr_de->name, name) == 0) {
            res = curr_de;
            break;
        }
        curr = curr->next;
    }
    return res;
}


struct dentry *named(const char *path)
{
    char name[DIRSIZ];
    struct dentry *de, *tmp;
    struct inode *inode;

    if (!path || *path == '\0')
        return NULL;

    de = (*path == '/') ? current_task->root : current_task->cwd;

    while ((path = skipelem(path, name)) != NULL)
    {
        if (!S_ISDIR(de->inode->mode))
            return NULL;

        if (strcmp(name, ".") == 0) {
            continue;
        } else if (strcmp(name, "..") == 0) {
            if (strcmp(de->name, "/") == 0)
                de = follow_up(de);
            de = de->parent;
        } else {
            if (de->mounted)
                de = follow_down(de);
            tmp = dentry_lookup(de, name);
            if (tmp != NULL)
                de = tmp;
            else if (de->inode != NULL) {
                inode = vfs_lookup(de->inode, name);
                if (inode == NULL)
                    return NULL;
                de = dentry_create(name, de, de->ops);
                if (de == NULL)
                    return NULL;
                de->inode = inode;
            }
        }
    }
    if (S_ISDIR(de->inode->mode) && de->mounted)
        de = follow_down(de);
    return de;
}

struct inode *namei(const char *path)
{
    struct dentry *de;
    struct inode  *ino = NULL;

    de = named(path);
    if (de != NULL)
        ino = idup(de->inode);
    return ino;
}



int vfs_init(void)
{
    slab_cache_init(&inode_cache, "inode-cache", sizeof(struct inode),
            0, 0, NULL, NULL);

    slab_cache_init(&file_cache, "file-cache", sizeof(struct file),
            0, 0, NULL, NULL);

    htable_init(inode_htable, INODE_HTABLE_BITS);

    list_init(&mounts);

    return 0;
}
