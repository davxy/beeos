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

#ifndef _LIMITS_H_
#define _LIMITS_H_

#define INT_MIN     0x80000000
#define INT_MAX     0x7FFFFFFF
#define UINT_MAX    0xFFFFFFFF

#define OPEN_MAX    32      /**< Number of open files */
#define NAME_MAX    64      /**< Chars in file name */
#define PATH_MAX    256     /**< Chars in a path */
#define ARG_MAX     1024    /**< Arguments and environment max length */
#define PIPE_BUF    512     /**< Bytes that can be written atomically */


#endif /* _LIMITS_H_ */
