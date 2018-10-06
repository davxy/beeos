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

#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_

#include <time.h>
#include <sys/types.h>

/** File types. @{ */
#define S_IFMT      0170000 /**< File type mask. */
#define S_IFIFO     0010000 /**< FIFO file. */
#define S_IFCHR     0020000 /**< Character device file. */
#define S_IFDIR     0040000 /**< Directory. */
#define S_IFBLK     0060000 /**< Block device file. */
#define S_IFREG     0100000 /**< Regular file. */
#define S_IFLNK     0120000 /**< Symbolic link. */
#define S_IFSOCK    0140000 /**< Socket. */
/** @} */

/** Owner access permissions. @{ */
#define S_IRWXU     0000700 /**< Owner access permissions mask. */
#define S_IRUSR     0000400 /**< Owner read access. */
#define S_IWUSR     0000200 /**< Owner write access. */
#define S_IXUSR     0000100 /**< Owner execute access. */
/** @} */

/** Group access permissions. @{ */
#define S_IRWXG     0000070 /**< Group access permissions mask. */
#define S_IRGRP     0000040 /**< Group read access. */
#define S_IWGRP     0000020 /**< Group write access. */
#define S_IXGRP     0000010 /**< Group execute access. */
/** @} */

/** Others access permissions. @{ */
#define S_IRWXO     0000007 /**< Others access permissions mask. */
#define S_IROTH     0000004 /**< Others read access. */
#define S_IWOTH     0000002 /**< Others write access. */
#define S_IXOTH     0000001 /**< Others execute access. */
/** @} */

/** S-bit. @{ */
#define S_ISUID     0004000 /**< S-UID. */
#define S_ISGID     0002000 /**< S-GID. */
#define S_ISVTX     0001000 /**< Sticky. */
/** @} */

/** Macroinstructions to verify the type of file. @{ */
/** Block device. */
#define S_ISBLK(m)   (((m) & S_IFMT) == S_IFBLK)
/** Character device. */
#define S_ISCHR(m)   (((m) & S_IFMT) == S_IFCHR)
/** FIFO. */
#define S_ISFIFO(m)  (((m) & S_IFMT) == S_IFIFO)
/** Regular file. */
#define S_ISREG(m)   (((m) & S_IFMT) == S_IFREG)
/** Directory. */
#define S_ISDIR(m)   (((m) & S_IFMT) == S_IFDIR)
/** Symbolic link. */
#define S_ISLNK(m)   (((m) & S_IFMT) == S_IFLNK)
/** Socket. */
#define S_ISSOCK(m)  (((m) & S_IFMT) == S_IFSOCK)
/** @} */

/** File stat structure */
struct stat {
    dev_t       st_dev;         /** Device containing the file */
    ino_t       st_ino;         /** inode number */
    mode_t      st_mode;        /** File type and permissions */
    nlink_t     st_nlink;       /** Number of hard links */
    uid_t       st_uid;         /** User ID of owner */
    gid_t       st_gid;         /** Group ID of owner */
    dev_t       st_rdev;        /** Device number (if special file) */
    off_t       st_size;        /** Total size, in bytes */
    time_t      st_atime;       /** Last access time */
    time_t      st_mtime;       /** Last modification time */
    time_t      st_ctime;       /** Last inode modification time. */
    blksize_t   st_blksize;     /** Block size for filesystem I/O */
    blkcnt_t    st_blocks;      /** Number of blocks allocated */
};

int stat(const char *path, struct stat *buf);

#endif /* _SYS_STAT_H_ */
