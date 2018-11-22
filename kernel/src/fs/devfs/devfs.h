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


#ifndef BEEOS_FS_DEVFS_H_
#define BEEOS_FS_DEVFS_H_

#include "fs/vfs.h"

struct super_block *devfs_super_create(dev_t dev);

ssize_t devfs_read(dev_t dev, void *buf, size_t size, size_t off);

ssize_t devfs_write(dev_t dev, const void *buf, size_t size, size_t off);

struct super_block *devfs_sb_get(void);


#endif /* BEEOS_FS_DEVFS_H_ */
