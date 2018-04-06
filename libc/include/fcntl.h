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

#ifndef _FCNTL_H_
#define _FCNTL_H_

/**
 * Main access modes.
 * @{
 */
#define O_ACCMODE       03       /**< Access mode mask */
#define O_RDONLY        00       /**< Read access */
#define O_WRONLY        01       /**< Write access */
#define O_RDWR          02       /**< Read/write access */
#define O_CREAT         0100     /**< Create if not exists (not fcntl) */
#define O_TRUNC         01000    /**< Truncate if exists (not fcntl) */
#define O_APPEND        02000    /**< Append if exists */
#define O_NONBLOCK      04000    /**< Open in non blocking mode (read/write) */
#define O_CLOEXEC       010000   /**< Close on exec */
#define O_NOCTTY        020000   /**< Do not allocate controlling terminal */
/** @} */

/**
 * Flags to be set with
 *   fcntl(fd, F_SETFD, ...)
 */
#define FD_CLOEXEC      1

#endif /* _FCNTL_H_ */
