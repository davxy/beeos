/*
 * Copyright 2015-2018 The BeeOS Authors. All rights reserved.
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

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    const unsigned char *src8 = (const unsigned char *)src;

    /* Check family type: only IPv4 is available here. */
    if (af != AF_INET) {
        errno = EAFNOSUPPORT;
        return NULL;
    }
    /* Check for NULL pointers. */
    if (src == NULL || dst == NULL) {
        errno = EINVAL;
        return NULL;
    }

    snprintf(dst, (size_t)size, "%u.%u.%u.%u",
            src8[0], src8[1], src8[2], src8[3]);
    return dst;
}
