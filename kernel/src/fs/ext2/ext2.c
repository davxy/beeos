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

#include "ext2.h"
#include "fs/vfs.h"
#include "fs/devfs/devfs.h"
#include "kmalloc.h"
#include "dev.h"
#include "util.h"
#include "panic.h"
#include <errno.h>
#include <string.h>
#include <stdint.h>


#define EXT2_MAGIC          0xef53
#define EXT2_NDIR_BLOCKS    12  /* Number of direct blocks */
#define EXT2_BLK_IND        12  /* Indirect blocks index */
#define EXT2_BLK_DBL        13  /* Double indirect blocks index */
#define EXT2_BLK_TPL        14  /* Triple indirect blocks index */

/*
 * Unused macros reserved for future extensions or completeness.
 */

#define EXT2_BAD_INO            1
#define EXT2_ROOT_INO           2
#define EXT2_ACL_IDX_INO        3
#define EXT2_ACL_DATA_INO       4
#define EXT2_BOOT_LOADER_INO    5
#define EXT2_UNDEL_DIR_INO      6


struct ext2_disk_super_block {
    uint32_t inodes_count;      /* Count of inodes in fs */
    uint32_t blocks_count;      /* Count of blocks in fs */
    uint32_t r_blocks_count;    /* Count of # of reserved blocks */
    uint32_t free_blocks_count; /* Count of # of free blocksw */
    uint32_t free_inodes_count; /* Count of # of free inodes */
    uint32_t first_data_block;  /* First block that contains data */
    uint32_t log_block_size;    /* Indicator of block size */
    int32_t  log_frag_size;     /* Indicator of the size of fragments */
    uint32_t blocks_per_group;  /* Count of # of blocks in each block group */
    uint32_t frags_per_group;   /* Count of # of fragments in each block group */
    uint32_t inodes_per_group;  /* Count of # of inodes in each blcok group */
    uint32_t mtime;             /* time filesystem was last mounted */
    uint32_t wtime;             /* time filesystem was last written to */
    uint16_t mnt_count;         /* number of times the fs has been mounted */
    int16_t  max_mnt_count;     /* number of times the fs can be mounted */
    uint16_t magic;             /* EXT2 Magic number */
    uint16_t state;             /* flags indicating current state of fs */
    uint16_t errors;            /* flags indicating errors */
    uint16_t pad;               /* padding */
    uint32_t lastcheck;         /* time the fs was last checked */
    uint32_t checkinterval;     /* maximum time between checks */
    uint32_t creator_os;        /* indicator of which OS created */
    uint32_t rev_level;         /* EXT2 revision level */
    uint32_t reserved[236];     /* padding to 1024 bytesOS */
};

struct ext2_group_desc {
    uint32_t block_bitmap;  /* address of block containing the block bitmap for this group */
    uint32_t inode_bitmap;  /* address of block containing the inode bitmap for this group */
    uint32_t inode_table;   /* address of the block containing the inode table for this group */
    uint16_t free_blocks_count; /* count of free blocks in group */
    uint16_t free_inodes_count; /* count of free inodes in group */
    uint16_t used_dirs_count;   /* number of inodes in this group that are directories */
    uint16_t pad;
    uint32_t reserved[3];
};

struct ext2_disk_inode {
    uint16_t mode;          /* File mode */
    uint16_t uid;           /* Owner UID */
    uint32_t size;          /* size in bytes */
    uint32_t atime;         /* access time */
    uint32_t ctime;         /* creation time */
    uint32_t mtime;         /* modification time */
    uint32_t dtime;         /* deletion time */
    uint16_t gid;           /* Group ID */
    uint16_t links_count;   /* links count */
    uint32_t blocks;        /* blocks count */
    uint32_t flags;         /* file flags */
    uint32_t reserved1;
    uint32_t block[15];     /* pointers to blocks */
    uint32_t version;
    uint32_t file_acl;      /* file ACL */
    uint32_t dir_acl;       /* directory acl */
    uint8_t  faddr;         /* fragment address */
    uint8_t  fsize;         /* fragment size */
    uint16_t pad1;
    uint32_t reserved2[3];
};

struct ext2_disk_dirent {
    uint32_t ino;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
    char     name[255];
};

struct ext2_super_block {
    struct super_block      base;
    uint32_t                block_size;
    uint32_t                inodes_per_group;
    uint32_t                log_block_size;
    struct ext2_group_desc *gd_table;
};

struct ext2_inode {
    struct inode base;
    uint32_t blocks[15]; /* pointers to blocks */
};



static int offset_to_block(size_t offset, const struct ext2_inode *inod,
                           const struct ext2_super_block *sb)
{
    uint32_t triple_block, double_block, indirect_block, block;
    uint8_t ind, dbl, tpl;
    uint32_t *buf;
    uint32_t shift;

    shift = 10 + sb->log_block_size;
    if (shift >= 8 * sizeof(size_t))
        return -1;

    /* Is direct? */
    if (offset < EXT2_NDIR_BLOCKS * sb->block_size) {
        ind = offset >> shift;
        if (ind >= sizeof(inod->blocks)/sizeof(inod->blocks[0]))
            return -1;
        return inod->blocks[ind];
    }

    buf = (uint32_t *)kmalloc(sb->block_size, 0);

    indirect_block = inod->blocks[EXT2_BLK_IND];
    double_block = inod->blocks[EXT2_BLK_DBL];
    triple_block = inod->blocks[EXT2_BLK_TPL];

    offset = (offset >> shift) - EXT2_NDIR_BLOCKS;
    ind = offset;
    dbl = offset >> 8;
    tpl = offset >> 16;

    if (tpl != 0)
        panic("ext2: required triple block %d", triple_block);

    if (dbl != 0)
        panic("ext2: required double block %d", double_block);

    if (devfs_read(sb->base.dev, buf, sb->block_size,
                   indirect_block*sb->block_size) != sb->block_size)
        return -1;
    block = buf[ind];

    kfree(buf, sb->block_size);

    return block;
}

/******************************************************************************
 *  Dentry operations
 ******************************************************************************/

static ssize_t ext2_read(struct ext2_inode *inod, void *buf,
                         size_t count, size_t off)
{
    const struct ext2_super_block *sb;
    int left;
    int block;
    size_t ext2_off, block_off, file_off;
    ssize_t n;

    sb = (struct ext2_super_block *)inod->base.sb;

    if (inod->base.size < off)
        return 0; /* EOF */
    if (inod->base.size < off + count)
        count = inod->base.size - off;

    file_off = off;
    left = count;
    while (left > 0) {
        block = offset_to_block(file_off, inod, sb);
        if (block < 0)
            break;
        block_off = off % sb->block_size; /* used just by the first block */
        ext2_off = block * sb->block_size + block_off;
        n = MIN(left, sb->block_size - block_off);
        if (devfs_read(sb->base.dev, buf, n, ext2_off) != n)
            break;

        left -= n;
        file_off += n;
        buf = (char *)buf + n;
        off = 0;
    }
    return count-left;
}

static struct inode *ext2_lookup(struct inode *dir, const char *name)
{
    struct ext2_disk_dirent *curr;
    struct ext2_disk_dirent *dirbuf;
    int count;
    struct inode *inod = NULL;

    dirbuf = (struct ext2_disk_dirent *)kmalloc(dir->size, 0);
    if (dirbuf == NULL)
        return NULL;

    if (devfs_read(dir->sb->dev, dirbuf, dir->size,
                   ((struct ext2_inode *)dir)->blocks[0] * 1024) != dir->size)
        goto end;

    count = dir->size;
    curr = dirbuf;
    while(count >= sizeof(struct ext2_disk_dirent)) {
        /* dirent->name is not null terminated */
        if(strlen(name) == (size_t)curr->name_len &&
           memcmp(name, curr->name, curr->name_len) == 0) {
            inod = iget(dir->sb, curr->ino);
            if (inod != NULL)
                inod->ref--; /* iget incremented the counter... release it */
            break;
        }
        if((curr->rec_len) == 0)
            break;
        count -= curr->rec_len;
        curr = (struct ext2_disk_dirent *)((char *)curr + curr->rec_len);
    }
end:
    kfree(dirbuf, dir->size);
    return inod;
}


static const struct inode_ops ext2_inode_ops = {
    .read   = (inode_read_t)ext2_read,
    .lookup = ext2_lookup,
};


/******************************************************************************
 *  Dentry operations
 ******************************************************************************/

static int ext2_readdir(struct inode *dir, unsigned int i,
                        struct dirent *dent)
{
    struct ext2_disk_dirent *curr;
    struct ext2_disk_dirent *dirbuf;
    size_t count, n;
    int ret = -1;

    dirbuf = (struct ext2_disk_dirent *)kmalloc(dir->size, 0);
    if (dirbuf == NULL)
        return -ENOMEM;

    ret = devfs_read(dir->sb->dev, dirbuf, dir->size,
                    ((struct ext2_inode *)dir)->blocks[0] * 1024);
    if (ret != dir->size) {
        if (ret >= 0)
            ret = -EIO;
        goto end;
    }

    count = dir->size;
    curr = dirbuf;
    n = 0;
    while(count >= sizeof(struct ext2_disk_dirent)) {
        if (n++ == i) {
            n = MIN(curr->name_len, NAME_MAX);
            memcpy(&dent->d_name, curr->name, n);
            dent->d_name[n] = '\0';
            dent->d_ino = curr->ino;
            ret = 0;
            break;
        }
        if((curr->rec_len) == 0)
            break;
        count -= curr->rec_len;
        curr = (struct ext2_disk_dirent *)((char *)curr + curr->rec_len);
    }

end:
    kfree(dirbuf, dir->size);
    return ret;
}


static int ext2_dentry_readdir(struct dentry *dir, unsigned int i,
                               struct dirent *dent)
{
    return ext2_readdir(dir->inod, i, dent);
}

static const struct dentry_ops ext2_dentry_ops = {
    .readdir = ext2_dentry_readdir
};



/******************************************************************************
 *  Superblock operations
 ******************************************************************************/


static struct inode *ext2_super_inode_alloc(struct super_block *sb)
{
    struct inode *inod;

    inod = (struct inode *)kmalloc(sizeof(struct ext2_inode), 0);
    if (inod != NULL)
        memset(inod, 0, sizeof(*inod));
    return inod;
}

static void ext2_super_inode_free(struct inode *inod)
{
    kfree(inod, sizeof(struct ext2_inode));
}

/*
 * Fetch inode information from the device.
 */
static int ext2_super_inode_read(struct inode *inod)
{
    int n;
    struct ext2_disk_inode disk_inod;
    const struct ext2_super_block *sb = (struct ext2_super_block *) inod->sb;
    int group = ((inod->ino - 1) / sb->inodes_per_group);
    const struct ext2_group_desc *gd = &sb->gd_table[group];
    int table_index = (inod->ino - 1 ) % sb->inodes_per_group;
    int blockno = ((table_index * 128) / 1024 ) + gd->inode_table;
    int ind = table_index % (1024 /128);

    n = devfs_read(sb->base.dev, &disk_inod, sizeof(disk_inod),
                   blockno * 1024 + ind*sizeof(disk_inod));
    if (n != sizeof(disk_inod))
        return -1;

    inod->ops = &ext2_inode_ops;
    inod->mode = disk_inod.mode;
    inod->uid = disk_inod.uid;
    inod->gid = disk_inod.gid;
    if (S_ISCHR(inod->mode) || S_ISBLK(inod->mode))
        inod->rdev  = disk_inod.block[0];
    inod->size = disk_inod.size;
    inod->atime = disk_inod.atime;
    inod->mtime = disk_inod.mtime;
    inod->ctime = disk_inod.ctime;

#if 0
    inod->blksize = 512; /* sure ??? */
    inod->blocks = (disk_inod.size-1)/inod->blksize+1;
#endif

    memcpy(((struct ext2_inode *)inod)->blocks, disk_inod.block,
            sizeof(disk_inod.block));

    return 0;
}


static const struct super_ops ext2_sb_ops =
{
    .inode_alloc = ext2_super_inode_alloc,
    .inode_free  = ext2_super_inode_free,
    .inode_read  = ext2_super_inode_read,
};


/*
 * TODO : rollback on error
 */
struct super_block *ext2_super_create(dev_t dev)
{
    size_t n;
    struct ext2_super_block *sb;
    struct inode *iroot;
    struct dentry *droot;
    unsigned int num_groups;
    struct ext2_disk_super_block dsb;
    uint32_t gd_block;

    if (devfs_read(dev, &dsb, sizeof(dsb), 1024) != sizeof(dsb))
        return NULL;

    if (dsb.magic != EXT2_MAGIC)
        return NULL;

    sb = (struct ext2_super_block *)kmalloc(sizeof(struct ext2_super_block), 0);
    if (sb == NULL)
        return NULL;

    sb->inodes_per_group = dsb.inodes_per_group;
    sb->base.dev = dev;
    sb->log_block_size = dsb.log_block_size;
    sb->block_size = 1024 << dsb.log_block_size;
    gd_block = (dsb.log_block_size == 0) ? (uint32_t)3 : (uint32_t)2;
    num_groups = (dsb.blocks_count - 1) / dsb.blocks_per_group + 1;

    n = sizeof(struct ext2_group_desc) * num_groups;
    sb->gd_table = (struct ext2_group_desc *)kmalloc(n, 0);
    if (sb->gd_table == NULL)
        return NULL;

    if (devfs_read(dev, sb->gd_table, n, sb->block_size*(gd_block-1)) != n)
        return NULL;

    droot = dentry_create("/", NULL, &ext2_dentry_ops);
    super_init(&sb->base, dev, droot, &ext2_sb_ops);

    /* Now that we can read inodes, we cache the root inode */
    iroot = inode_create(&sb->base, EXT2_ROOT_INO, S_IFDIR, &ext2_inode_ops);
    droot->inod = idup(iroot);

    return &sb->base;
}
