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

#ifndef _BEEOS_FS_EXT2_H_
#define _BEEOS_FS_EXT2_H_

#include "fs/vfs.h"
#include <stdint.h>


struct ext2_disk_super_block
{
    uint32_t inodes_count; //Count of inodes in fs
    uint32_t blocks_count; //Count of blocks in fs
    uint32_t r_blocks_count; //Count of # of reserved blocks
    uint32_t free_blocks_count; //Count of # of free blocksw
    uint32_t free_inodes_count; //Count of # of free inodes
    uint32_t first_data_block; //First block that contains data
    uint32_t log_block_size; //Indicator of block size
    int32_t  log_frag_size; //Indicator of the size of fragments
    uint32_t blocks_per_group;//Count of # of blocks in each block group
    uint32_t frags_per_group;//Count of # of fragments in each block group
    uint32_t inodes_per_group;//Count of # of inodes in each blcok group
    uint32_t mtime; //time filesystem was last mounted
    uint32_t wtime;//time filesystem was last written to
    uint16_t mnt_count;//number of times the file system has been mounted
    int16_t  max_mnt_count;//number of times the file system can be mounted
    uint16_t magic;//EXT2 Magic number
    uint16_t state;//flags indicating current state of filesystem
    uint16_t errors;//flags indicating errors
    uint16_t pad;//padding
    uint32_t lastcheck;//time the fs was last checked
    uint32_t checkinterval;//maximum time between checks
    uint32_t creator_os;//indicator of which OS created 
    uint32_t rev_level;//EXT2 revision level
    uint32_t reserved[236];//padding to 1024 bytesOS
};

struct ext2_group_desc
{
    uint32_t block_bitmap;//address of block containing the block bitmap for this group
    uint32_t inode_bitmap;//address of block containing the inode bitmap for this group
    uint32_t inode_table;//address of the block containing the inode table for this group
    uint16_t free_blocks_count;//count of free blocks in group
    uint16_t free_inodes_count;//count of free inodes in group
    uint16_t used_dirs_count; //number of inodes in this group that are directories
    uint16_t pad;
    uint32_t reserved[3];
};

struct ext2_disk_inode
{
    uint16_t mode;          //File mode
    uint16_t uid;           //Owner UID
    uint32_t size;          //size in bytes
    uint32_t atime;         //access time
    uint32_t ctime;         //creation time
    uint32_t mtime;         //modification time
    uint32_t dtime;         //deletion time
    uint16_t gid;           //Group ID
    uint16_t links_count;   //links count
    uint32_t blocks;        //blocks count
    uint32_t flags;         //file flags
    uint32_t reserved1;
    uint32_t block[15];     //pointers to blocks
    uint32_t version;
    uint32_t file_acl;      //file ACL
    uint32_t dir_acl;       //directory acl
    uint8_t  faddr;         //fragment address
    uint8_t  fsize;         //fragment size
    uint16_t pad1;
    uint32_t reserved2[3];
};

struct ext2_disk_dirent
{
    uint32_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
    char     name[255];
};

#define EXT2_BAD_INO            1
#define EXT2_ROOT_INO           2
#define EXT2_ACL_IDX_INO        3
#define EXT2_ACL_DATA_INO       4
#define EXT2_BOOT_LOADER_INO    5
#define EXT2_UNDEL_DIR_INO      6


#if 0
struct ext2_aux
{
    uint32_t size;
    uint32_t gd_block;
    uint32_t block_size;
    ext2_group_descriptor_t *gd_table; 
    uint32_t *block_bitmap;
    uint32_t *inode_bitmap;
};

struct ext2_fs 
{
    vfs_sb_t *superblock;
    char name[10];
    ext2_aux_t *aux;
    uint16_t dev;
    vfs_ops_t *ops;
};

#endif


struct super_block *ext2_super_create(dev_t dev);

#endif /* _BEEOS_FS_EXT2_H_ */
