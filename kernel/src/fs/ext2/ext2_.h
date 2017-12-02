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

/* inodes are indexed starting at 1 */
#include <fs/vfs.h>
#define INODE(x) (x-1)
#define BLOCK(x) (1024*(x))
#define EXT2_MAGIC 0xef53
typedef struct ext2_superblock {
	uint32_t s_inodes_count; //Count of inodes in fs
	uint32_t s_blocks_count; //Count of blocks in fs
	uint32_t s_r_blocks_count; //Count of # of reserved blocks
	uint32_t s_free_blocks_count; //Count of # of free blocksw
	uint32_t s_free_inodes_count; //Count of # of free inodes
	uint32_t s_first_data_block; //First block that contains data
	uint32_t s_log_block_size; //Indicator of block size
	int32_t  s_log_frag_size; //Indicator of the size of fragments
	uint32_t s_blocks_per_group;//Count of # of blocks in each block group
	uint32_t s_frags_per_group;//Count of # of fragments in each block group
	uint32_t s_inodes_per_group;//Count of # of inodes in each blcok group
	uint32_t s_mtime; //time filesystem was last mounted
	uint32_t s_wtime;//time filesystem was last written to
	uint16_t s_mnt_count;//number of times the file system has been mounted
	int16_t  s_max_mnt_count;//number of times the file system can be mounted
	uint16_t s_magic;//EXT2 Magic number
	uint16_t s_state;//flags indicating current state of filesystem
	uint16_t s_errors;//flags indicating errors
	uint16_t s_pad;//padding
	uint32_t s_lastcheck;//time the fs was last checked
	uint32_t s_checkinterval;//maximum time between checks
	uint32_t s_creator_os;//indicator of which OS created 
	uint32_t s_rev_level;//EXT2 revision level
	uint32_t s_reserved[236];//padding to 1024 bytesOS
} __attribute__((packed)) ext2_superblock_t;

typedef struct ext2_group_descriptor {
	uint32_t bg_block_bitmap;//address of block containing the block bitmap for this group
	uint32_t bg_inode_bitmap;//address of block containing the inode bitmap for this group
	uint32_t bg_inode_table;//address of the block containing the inode table for this group
	uint16_t bg_free_blocks_count;//count of free blocks in group
	uint16_t bg_free_inodes_count;//count of free inodes in group
	uint16_t bg_used_dirs_count; //number of inodes in this group that are directories
	uint16_t bg_pad;
	uint32_t bg_reserved[3];
} __attribute__((packed)) ext2_group_descriptor_t;


struct ext2_inode {
	uint16_t i_mode;//File mode
	uint16_t i_uid;//Owner UID
	uint32_t i_size;//size in bytes
	uint32_t i_atime;//access time
	uint32_t i_ctime;//creation time
	uint32_t i_mtime;//modification time
	uint32_t i_dtime;//deletion time
	uint16_t i_gid;//Group ID
	uint16_t i_links_count;//links count
	uint32_t i_blocks;//blocks count
	uint32_t i_flags;//file flags
	uint32_t i_reserved1;
	uint32_t i_block[15];//pointers to blocks
	uint32_t i_version;
	uint32_t i_file_acl;//file ACL
	uint32_t i_dir_acl;//directory acl
	uint8_t i_faddr;//fragment address
	uint8_t i_fsize;//fragment size
	uint16_t i_pad1;
	uint32_t i_reserved2[3];
}__attribute__((packed));
typedef struct ext2_inode ext2_inode_t;

typedef struct ext2_directory {
	uint32_t inode;
	uint16_t rec_len;
	uint8_t name_len;
	uint8_t file_type;
	char name[255];
} ext2_directory_t;

#define EXT2_BAD_INO 			1
#define EXT2_ROOT_INO			2
#define EXT2_ACL_IDX_INO		3
#define EXT2_ACL_DATA_INO		4
#define EXT2_BOOT_LOADER_INO	5
#define EXT2_UNDEL_DIR_INO		6

typedef struct ext2_aux {
	uint32_t size;
	uint32_t gd_block;
	uint32_t block_size;
//	uint32_t fuckass[0];
	ext2_group_descriptor_t *gd_table; 
	//may cache these
	uint32_t *block_bitmap;
	uint32_t *inode_bitmap;
} ext2_aux_t;
/*
struct vfs_fs {
	vfs_sb_t *superblock;
	char name[10];
	void *fs;
	vfs_ops_t *ops;
};
*/
typedef struct ext2_fs {
	vfs_sb_t *superblock;
	char name[10];
	ext2_aux_t *aux;
	uint16_t dev;
	vfs_ops_t *ops;
} ext2_fs_t;
int ext2_read_superblock(vfs_fs_t *fs, uint16_t dev);
struct inode * ext2_load_inode(ext2_fs_t *fs, int ino);

struct inode * ext2_namei(struct inode *dir, char *file);
size_t ext2_read_inode(struct inode *inode,
	void *_buf, size_t length, 
	off_t offset);
size_t ext2_write_inode(struct inode *inode,
	void *_buf, size_t length, 
	off_t offset);

