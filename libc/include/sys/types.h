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

/**
 * @file    types.h
 * @brief
 * @author    Davide Galassi
 * @date    Jul 12, 2014
 */

#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

#include <stdint.h>

typedef unsigned int    size_t;
typedef int             ssize_t;

typedef int             pid_t;
typedef uint16_t        dev_t;
typedef uint16_t        ino_t;
typedef uint16_t        mode_t;
typedef unsigned int    nlink_t;
typedef unsigned int    uid_t;
typedef unsigned int    gid_t;
typedef unsigned int    id_t;
typedef long int        off_t;
typedef long int        blksize_t;
typedef long int        blkcnt_t;

#define makedev(maj, min)  ((((maj) & 0xFF) << 8) | ((min) & 0xFF))
#define major(dev) (((dev) >> 8) & 0xFF)
#define minor(dev) ((dev) & 0xFF)

#endif /* _SYS_TYPES_H_ */
