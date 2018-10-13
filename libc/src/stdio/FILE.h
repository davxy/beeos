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

#ifndef _STDIO_FILE_H_
#define _STDIO_FILE_H_

#define FILE_FLAG_ERROR     (1 << 0)
#define FILE_FLAG_EOF       (1 << 1)
#define FILE_FLAG_NFREE     (1 << 2)

struct _FILE {
    int     fd;         /* file descriptor */
    int     flags;      /* mode of file access */
    int     bufmode;    /* buffering mode */
    int     bufsize;    /* buffer size */
    int     nr;         /* buffered read characters */
    int     nw;         /* buffered write characters */
    char    *ptr;       /* next character position */
    char    *buf;       /* location of buffer */
};

#endif /*_STDIO_FILE_H_ */
