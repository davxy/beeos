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

uint32_t htonl(uint32_t hostlong)
{
    union {
        uint32_t val;
        uint8_t  raw[4];
    } dst;

    dst.raw[0] = (uint8_t)((hostlong >> 24) & 0xFF);
    dst.raw[1] = (uint8_t)((hostlong >> 16) & 0xFF);
    dst.raw[2] = (uint8_t)((hostlong >> 8)  & 0xFF);
    dst.raw[3] = (uint8_t) (hostlong & 0xFF);
    return dst.val;
}
