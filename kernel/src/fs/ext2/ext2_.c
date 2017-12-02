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

#include <kernel/common.h>
#include <kernel/memory.h>
#include <fs/vfs.h>
#include <mm/liballoc.h>
#include "ext2fs_defs.h"
#include "ext2fs.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#define BLOCK_SIZE 512
#define EXT2_SB_SIZE 1024
#define EXT2_SB_BLOCK 2
#define EXT2_INO_BLK_DEV 0
#define EXT2_INO_BLK_IND 12
#define EXT2_INO_BLK_DBL 13
#define EXT2_INO_BLK_TPL 14


//moved debug functions to seperate file
#include "ext2_debug.c"

//FIXME
vfs_ops_t ext2_ops = {
	ext2_read_inode,//vfs_read_inode_t read;
	ext2_write_inode,//vfs_write_inode_t write;
	ext2_read_superblock,//vfs_read_sb_t read_sb;
	ext2_namei//vfs_namei_t namei;
};

void ext2_inode_to_vfs(ext2_fs_t *fs,struct inode *vfs,ext2_inode_t *ext2,uint32_t inode)
{
	struct stat *st = &vfs->info;
	st->st_dev = fs->dev;
	st->st_ino = inode;
	st->st_mode = ext2->i_mode;
	st->st_nlink = ext2->i_links_count;
	st->st_uid = ext2->i_uid;
	st->st_gid = ext2->i_gid;

	if(ext2->i_mode & S_IFCHR || ext2->i_mode & S_IFBLK)
	{
		st->st_rdev = ext2->i_block[0];
	}

	st->st_size = ext2->i_size;
	st->st_atime = ext2->i_atime;
	st->st_ctime = ext2->i_ctime;
//	st->st_dtime = ext2->i_dtime;
	st->st_mtime = ext2->i_mtime;
	st->st_blksize = 512;
	st->st_blocks = ext2->i_size / st->st_blksize;
	if(ext2->i_size % st->st_blksize)
		st->st_blocks++;


	//TODO:if part of mount point,keep in cache
	vfs->flags = 0;
	vfs->storage = ext2;
	vfs->fs = (vfs_fs_t *)fs;
}

size_t ext2_read_block(ext2_fs_t *fs, void *buf, int block)
{
	return	read_block(fs->dev, buf, block, fs->aux->block_size);
}

size_t ext2_write_block(ext2_fs_t *fs, void *buf, int block)
{
	return	write_block(fs->dev, buf, block, fs->aux->block_size);
}

//FIXME: fix error handling to catch all memory leaks
int ext2_read_superblock(vfs_fs_t *fs, uint16_t dev)
{
	struct inode *root = NULL;
	int count;
	ext2_fs_t *ext2 = (ext2_fs_t *)fs;
	ext2_superblock_t *sb;

	fs->dev = dev;

	fs->superblock->sb = kcalloc(sizeof(ext2_superblock_t), 1);
	sb = fs->superblock->sb;

	count = read_block_at(fs->dev, sb, 1,1024, 0, 1024);

	if(count != 1024)
	{
		printf("reading superblock failed\n");
		goto fail;
	}

	if(sb->s_magic != EXT2_MAGIC)
	{
		printf("bad magic number: %X\n", sb->s_magic);
		goto fail;
	}

	ext2->aux->gd_block = (sb->s_log_block_size == 0 ? 3 : 2);

	int num_groups = (sb->s_blocks_count - 1)/sb->s_blocks_per_group + 1;

//	int size = sizeof(ext2_group_descriptor_t)*num_groups;

//	printf("EXT2 groups %i blocks %i gd_size %i\n",
//		num_groups,sb->s_blocks_count, size);

	ext2->aux->gd_table =
		(ext2_group_descriptor_t *)kcalloc(sizeof(ext2_group_descriptor_t),num_groups);

	ext2->aux->block_size = 1024 << sb->s_log_block_size;

	ext2_read_block(ext2, ext2->aux->gd_table, ext2->aux->gd_block - 1);

	//now that we can read inodes, we cache the root inode(2)
	//in the vfs superblock struct
	if((root  = kcalloc(sizeof(*root), 1)) == NULL)
		goto fail;

	if((root = ext2_load_inode((ext2_fs_t *)fs, EXT2_ROOT_INO)) == NULL)
		goto fail;

	fs->superblock->root = root;

	return 0;
fail:
	if(root != NULL)
		kfree(root);
	return -1;
}

struct inode * ext2_load_inode(ext2_fs_t *fs, int inode)
{
	struct inode *read = kcalloc(sizeof(*read), 1);
	ext2_inode_t *ext2_ino = kcalloc(sizeof(*ext2_ino),1);
	ext2_inode_t *inode_block = kcalloc(sizeof(*ext2_ino),8);
	ext2_superblock_t *sb;
	int ind;
	if(fs == NULL)
		return NULL;
	if(fs->superblock == NULL)
	return NULL;

	sb = fs->superblock->sb;
	if(sb == NULL)
		return NULL;
	int group = ((inode - 1)/sb->s_inodes_per_group);

	ext2_group_descriptor_t * gd = &fs->aux->gd_table[group];
//	gd_print(*gd);
	int table_index = ((inode - 1 ) % sb->s_inodes_per_group) ;
//	printf("inode %i table_index = %i\n",inode, table_index);
	int block = ((table_index * 128) / 1024 )+ gd->bg_inode_table;
//	printf("BLOCK %i FUCK %i\n",block,((table_index * 128) % 1024) / 128);
	read_block(fs->dev, inode_block, block, 1024);
	ind = table_index % (1024 /128);
	kmemcpy(ext2_ino, &inode_block[ind], sizeof(*ext2_ino));
	//inode_print(*ext2_ino);
	kfree(inode_block);
	ext2_inode_to_vfs(fs, read,ext2_ino,inode);
	return read;
}
//read_dir
//typedef uint32_t ino_t;
struct inode * ext2_namei(struct inode *dir, char *file)
{
	ext2_fs_t *fs = (ext2_fs_t *)dir->fs;
	struct inode *inode = dir;//ext2_load_inode((ext2_fs_t *)dir->fs, dir->info.st_ino);
	ext2_inode_t * in = (ext2_inode_t *)(inode->storage);
	int len = 0;
	ext2_directory_t *free,*ext2_dir= kmalloc(inode->info.st_size);
	free = ext2_dir;
	int count =inode->info.st_size;
	void *fdir = ext2_dir;
	int ino_num;

//	inode_print(*in);
	ext2_read_block(fs, ext2_dir, in->i_block[0]);

//	ext2_read(&test, dir, 4096, 0);
	while(count)
	{
	//	print_dir_entry(ext2_dir);
	//	printf("%s %s %i\n", file, ext2_dir->name, ext2_dir->name_len);
		if(ext2_dir->name_len  == strlen(file) && !strncmp(ext2_dir->name,file,strlen(file)-1))
		{
			kfree(fdir);
		//	printf("loading inode %i\n",ext2_dir->inode);
			ino_num = ext2_dir->inode;
			kfree(free);
			return ext2_load_inode(fs,ino_num);//ext2_dir->inode);
		}
		if((len = ext2_dir[0].rec_len) == 0)
			break;
		ext2_dir = (ext2_directory_t *)(uint32_t)((uint32_t)ext2_dir + (uint32_t)ext2_dir->rec_len);
		count -= len;

	}
	return 0;
}
/*
 *(0-11):5762-5773, (IND):512, (12-15):5774-5777, (16-31):2161-2176, (32-63):2337-2368, (64-127):2593-2656,
 *(128-255):2689-2816, (256-267):3874-3885,
 *(DIND):543, (IND):539, (268-511):3886-4129, (512-523):4609-4620, (IND):540, (524-557):4621-4654
 *TOTAL: 562
 *

 * */
#define EXT2_NDIR_BLOCKS 12
#define EXT2_DIR_BLOCKS 12
int byte_to_block(ext2_fs_t *fs, ext2_inode_t *inode, size_t offset)
{
	uint32_t triple_block, double_block, indirect_block, block = 0;
	uint8_t ind, dbl, tpl;
	uint32_t *buf;
	uint32_t block_size = fs->aux->block_size;
	uint32_t shift = 10 + ((ext2_superblock_t *)fs->superblock->sb)->s_log_block_size;

	if(offset < EXT2_DIR_BLOCKS*block_size)
	{
		return inode->i_block[offset >> shift];
	}

	buf = kcalloc(1, block_size);

	indirect_block = inode->i_block[EXT2_INO_BLK_IND];
	double_block   = inode->i_block[EXT2_INO_BLK_DBL];
	triple_block   = inode->i_block[EXT2_INO_BLK_TPL];

	offset =  (offset >> shift) - EXT2_DIR_BLOCKS;

	ind = offset;
	dbl = offset >> 8;
	tpl = offset  >> 16;

	if(tpl)
	{
		if(ext2_read_block(fs, buf, triple_block) != block_size)
			return -1;
		double_block = buf[tpl - 1];
	}

	if(dbl)
	{
		if(ext2_read_block(fs, buf, double_block) != block_size)
			return -1;
		indirect_block = buf[dbl - 1];
	}

	if(ext2_read_block(fs, buf, indirect_block) != block_size)
		return -1;
	block = buf[ind];

	kfree(buf);
//	printf("New ind %x dbl %x tpl %x shifted %x block %i\n", ind, dbl, tpl, offset, block);

	return block;
}


int byte_to_block_old(ext2_fs_t *fs, ext2_inode_t *inode, size_t offset)
{
	uint32_t block_size = fs->aux->block_size;
	off_t block_offset = offset / block_size;
	uint32_t *indirect = NULL, *dbl_indirect = NULL, *tpl_indirect = NULL;
	int block = 0;
	unsigned offset2 = offset, range;
	uint8_t ind, dbl = 0, tpl = 0;
	if(offset > inode->i_size)
		return -1;
	printf("\n");
//	byte_to_block2(fs, inode, offset);

	range = EXT2_NDIR_BLOCKS*block_size;

	if(offset < range)
	{
		return inode->i_block[block_offset];
	}

	offset2 -= range;
	range	= (block_size / sizeof(uint32_t)) * block_size;

	indirect = kmalloc(block_size);

	if(offset2 < range)
	{
		if(ext2_read_block(fs, indirect, inode->i_block[EXT2_INO_BLK_IND]) != block_size)
			return -1;
		block_offset =  offset2 / block_size;
		block = indirect[block_offset];
		ind = block_offset;
		printf("Old ind %x dbl %x tpl %x shifted %x\n", ind, dbl, tpl, offset2/0x400);

		goto done;
	}

	offset2 -= range;
	range *= block_size;

	dbl_indirect = kmalloc(block_size);

	if(offset2 < range)
	{
		if(ext2_read_block(fs, dbl_indirect, inode->i_block[EXT2_INO_BLK_DBL]) != block_size)
			return -1;

		block_offset = offset2 / ((block_size / sizeof(uint32_t)) * block_size);
		dbl = block_offset;
		block = dbl_indirect[block_offset];

		offset2 -= (block_offset*block_size*(block_size/4));

		if(ext2_read_block(fs, indirect, block) != block_size)
			return -1;

		block_offset = (offset2)/ block_size;
		ind = block_offset;

		block = indirect[block_offset];
		printf("O1d ind %x dbl %x tpl %x shifted %x\n", ind, dbl, tpl, offset2/0x400);
		goto done;
	}

	PANIC("FUCK\n");
	offset2 -= range;
	range *= block_size;
	tpl_indirect = kmalloc(block_size);

	if(offset2 < range)
	{
		if(ext2_read_block(fs, tpl_indirect, inode->i_block[EXT2_INO_BLK_TPL]) != block_size)
			return -1;

		block_offset = offset2 / ((block_size * block_size / sizeof(uint32_t)) * block_size);

		block = tpl_indirect[block_offset];

		offset2 -= (block_offset*block_size);

		if(ext2_read_block(fs, dbl_indirect, block) != block_size)
			return -1;

		block_offset = offset2 / ((block_size / sizeof(uint32_t)) * block_size);

		block = dbl_indirect[block_offset];

		offset2 -= (block_offset*block_size);

		if(ext2_read_block(fs, indirect, block) != block_size)
			return -1;

		block_offset = offset2 / block_size;

		block = indirect[block_offset];

	}

done:
	if(indirect != NULL)
		kfree(indirect);
	if(dbl_indirect != NULL)
		kfree(dbl_indirect);
	if(tpl_indirect != NULL)
		kfree(tpl_indirect);


	return block;
}
//FIXME: better error detection, need to combine read and write and use
//		 the existing functions as wrappers to a functiont that has a
//		 rw boolean
size_t ext2_read_inode(struct inode *inode,void *_buf,
	size_t nbytes, off_t offset)
{
	ext2_fs_t *fs = (ext2_fs_t *)inode->fs;
	ext2_inode_t *ext2_ino = inode->storage;
	int block_size = fs->aux->block_size;

	uint8_t *buf = _buf;
	uint8_t *bounce = kmalloc(block_size);

	int remaining = nbytes;
	size_t count = 0;
	int block, block_ofs, cur_block_size,
		to_end, till_end, cur_size;
	while(remaining > 0)
	{
		block = byte_to_block(fs, ext2_ino,offset);
		//printf("reading block %i %i\n",block, offset);
		block_ofs = offset % block_size;
		cur_block_size = block_size - block_ofs;
		to_end = ext2_ino->i_size - offset;
		till_end = (to_end < cur_block_size) ? to_end: cur_block_size;

		cur_size = remaining < till_end ? remaining : till_end;
		if(cur_size <= 0)
			break;

		if(block_ofs == 0 && cur_size == block_size)
		{
			if(ext2_read_block(fs, buf + count, block)  == 0)
			{
				count = -1;
				goto done;
			}
		}
		else
		{
			if(ext2_read_block(fs, bounce, block) == 0)
			{
				count = -1;
				goto done;
			}

			kmemcpy(buf + count, bounce + block_ofs,cur_size);
		}

		count  += cur_size;
		offset += cur_size;
		remaining   -= cur_size;
	}

done:
	if(bounce != NULL)
		kfree(bounce);

	return count;
}
size_t ext2_write_inode(struct inode *inode,void *_buf,
	size_t nbytes, off_t offset)
{
	ext2_fs_t *fs = (ext2_fs_t *)inode->fs;
	ext2_inode_t *ext2_ino = inode->storage;
	int block_size = fs->aux->block_size;

	uint8_t *buf = _buf;
	uint8_t *bounce = kmalloc(block_size);

	int remaining = nbytes;
	size_t count = 0;
	int block, block_ofs, cur_block_size,
		to_end, till_end, cur_size;
	printf("Dass\n");
	if(nbytes + offset > ext2_ino->i_size)
	{
		printf("extensible files not yet supported\n");
		return 0;
	}
	while(remaining > 0)
	{
		block = byte_to_block(fs, ext2_ino,offset);
		block_ofs = offset % block_size;
		cur_block_size = block_size - block_ofs;
		to_end = ext2_ino->i_size - offset;
		till_end = (to_end < cur_block_size) ? to_end: cur_block_size;

		cur_size = remaining < till_end ? remaining : till_end;
		if(cur_size <= 0)
			break;

		if(block_ofs == 0 && cur_size == block_size)
		{
			if(ext2_write_block(fs, buf + count, block)  == 0)
			{
				count = -1;
				goto done;
			}
		}
		else
		{
			if(block_ofs > 0 || cur_size < cur_block_size)
			{
				ext2_read_block(fs, bounce, block);
			}
			kmemcpy(bounce + block_ofs,buf + count, cur_size);
			if(ext2_write_block(fs, bounce, block) == 0)
			{
				count = -1;
				goto done;
			}

		}

		count  += cur_size;
		offset += cur_size;
		remaining   -= cur_size;
	}

done:
	if(bounce != NULL)
		kfree(bounce);
	printf("shit\n");
	return count;
}


int ext2_init()
{
	printf("Initializing EXT2 FS\n");
	ext2_fs_t * new = (ext2_fs_t *)vfs_alloc();
	new->aux = kcalloc(sizeof(ext2_aux_t),1);
	new->ops = &ext2_ops;
	//FIXME:should use strncpy
	strcpy(new->name, "ext2");
	return vfs_register_fs((vfs_fs_t *)new);
}

