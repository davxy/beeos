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

#ifndef _DIRENT_H_
#define _DIRENT_H_

#include <sys/types.h>
#include <limits.h>     /* NAME_MAX */

/** Directory entry. */
struct dirent {
    ino_t   d_ino;                  /** Inode number */
    char    d_name[NAME_MAX+1];     /** Directory name */
};

typedef struct DIR {
    int    fdn;          /** Directory file descriptor */
    struct dirent dent;  /** Last directory entry */
} DIR;

DIR *opendir(const char *name);

int closedir(DIR *dirp);

struct dirent *readdir(DIR *dirp);

void rewinddir(DIR *dirp);

#endif /* _DIRENT_H_ */
