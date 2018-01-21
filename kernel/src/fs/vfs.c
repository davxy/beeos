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

// http://www.tldp.org/LDP/lki/lki-3.html

struct fs_type fs_list[] =
{
    { "ext2",   ext2_sb_create }
};

#define FS_LIST_LEN (sizeof(fs_list)/sizeof(fs_list[0]))

void sb_init(struct sb *sb, dev_t dev, struct inode *root,
        const struct sb_ops *ops)
{
    sb->dev = dev;
    sb->root = root;
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

#define KEY(dev,ino)    (((dev)<<16) + (ino))

int fs_init(void)
{
    slab_cache_init(&inode_cache, "inode-cache", sizeof(struct inode),
            0, 0, NULL, NULL);

    slab_cache_init(&file_cache, "file-cache", sizeof(struct file),
            0, 0, NULL, NULL);

    htable_init(inode_htable, INODE_HTABLE_BITS);
    
    return 0;
}

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
    struct inode *inode = slab_cache_alloc(&inode_cache, 0);
    return inode;
}


/**
 * The VFS maintains an inode cache to speed up accesses to all of the 
 * mounted file systems. Every time a VFS inode is read from the inode cache 
 * the system saves an access to a physical device.
 * The VFS inode cache is implmented as a hash table whose entries are pointers
 * to lists of VFS inode which have the same hash value. The hash value of an
 * inode is calculated from its inode number and from the device identifier for
 * the underlying physical device containing the file system. Whenever the VFS
 * needs to access an inode, it first looks in the VFS inode cache. 
 * To find an inode in the cache, the system first calculates its hash value 
 * and then uses it as an index into the inode hash table. This gives it a 
 * pointer to a list of inodes with the same hash value. It then reads each 
 * inode in turn until it finds one with both the same inode number and the 
 * same device identifier as the one that it is searching for.
 *
 * If it can find the inode in the cache, its count is incremented to show that
 * it has another user and the file system access continues. Otherwise a free 
 * VFS inode must be found so that the file system can read the inode from 
 * memory. VFS has a number of choices about how to get a free inode. If the 
 * system may allocate more VFS inodes then this is what it does; it allocates 
 * kernel pages and breaks them up into new, free, inodes and puts them into 
 * the inode list. All of the system's VFS inodes are in a list pointed at by 
 * first_inode  as well as in the inode hash table. If the system already has 
 * all of the inodes that it is allowed to have, it must find an inode that is
 * a good candidate to be reused. Good candidates are inodes with a usage count
 * of zero; this indicates that the system is not currently using them. Really 
 * important VFS inodes, for example the root inodes of file systems always
 * have a usage count greater than zero and so are never candidates for reuse. 
 * Once a candidate for reuse has been located it is cleaned up. 
 * The VFS inode might be dirty and in this case it needs to be written back 
 * to the file system or it might be locked and in this case the system must 
 * wait for it to be unlocked before continuing. The candidate VFS inode must 
 * be cleaned up before it can be reused.
 * However the new VFS inode is found, a file system specific routine must be 
 * called to to fill it out from information read from the underlying real file
 * system. Whilst it is being filled out, the new VFS inode has a usage count 
 * of one and is locked so that nothing else accesses it until it contains 
 * valid information.
 * To get the VFS inode that is actually needed, the file system may need to 
 * access several other inodes. This happens when you read a directory; only 
 * the inode for the final directory is needed but the inodes for the 
 * intermediate directories must also be read. As the VFS inode cache is used 
 * and filled up, the less used inodes will be discarded and the more used
 * inodes will remain in the cache.
 */

struct inode *inode_lookup(dev_t dev, ino_t ino)
{
    struct inode *ip;
    struct htable_link *lnk;
    lnk = htable_lookup(inode_htable, KEY(dev,ino), INODE_HTABLE_BITS);
    while (lnk != NULL)
    {
        ip = struct_ptr(lnk, struct inode, hlink);
        if (ip->ref > 0 && ip->dev == dev && ip->ino == ino)
        {
            ip->ref++;
            return ip;
        }
        lnk = lnk->next;
    }
    return NULL;
}

void inode_init(struct inode *inode, dev_t dev, ino_t ino)
{
    inode->dev = dev;
    inode->rdev = 0;
    inode->ino = ino;
    inode->ref = 1;
    inode->sb = NULL;
    htable_insert(inode_htable, &inode->hlink, KEY(dev,ino), INODE_HTABLE_BITS);
}

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


struct inode *fs_namei(const char *path)
{
    char name[DIRSIZ];
    struct inode *ip, *previp;

    if (!path || *path == '\0')
        return NULL;

    if (*path == '/')
        ip = idup(current_task->root);
    else
        ip = idup(current_task->cwd);

    while ((path = skipelem(path, name)) != NULL)
    {
        if (!S_ISDIR(ip->mode))
        {
            iput(ip);
            return NULL;
        }
        previp = ip;
        ip = fs_lookup(ip, name);
        iput(previp);
        if (ip == NULL)
            return NULL;
        idup(ip);
    }

    return ip;
}
