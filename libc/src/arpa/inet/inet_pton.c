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
#include <string.h>
#include <stdlib.h>

#define INET_PTON_MAX_STRING_SIZE 31

int inet_pton(int af, const char *src, void *dst)
{
    char *t;
    int ipv4[4];
    int i;
    char *dst8 = (char *)dst;
    char source[INET_PTON_MAX_STRING_SIZE + 1];

    /* Check family type: only IPv4 is available here. */
    if (af != AF_INET) {
        errno = EAFNOSUPPORT;
        return -1;
    }

    /* Check for NULL pointers. */
    if (src == NULL || dst == NULL) {
        errno = EINVAL;
        return -1;
    }

    /* Check the source string size. */
    if (strlen(src) > INET_PTON_MAX_STRING_SIZE) {
        /*
         * The IPv4 address scan is finished prematurely
         * return zero to tell that the address string is not correct.
         */
        return 0;
    }

    strcpy(source, src);

    /* Start tokenize the string */
    t = strtok (source, ".");
    for (i = 0; i < 4 && t != NULL; i++) {
        ipv4[i] = atoi(t);
        if (ipv4[i] > 255 || ipv4[i] < 0) {
            /*
             * An octet cannot have a value greater than 255, 
             * and cannot be negative.
             */
            break;
        }
        t = strtok(NULL, ".");
    }
    if (i < 4)
        return 0;

    /*
     * Translate into a network byte order IPv4 address:
     * the architecture is little-endian.
     */
    dst8[0] = ipv4[0];
    dst8[1] = ipv4[1];
    dst8[2] = ipv4[2];
    dst8[3] = ipv4[3];
    return 1;
}
