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

uint16_t ntohs(uint16_t netshort)
{
    union {
        uint16_t val;
        uint8_t  raw[2];
    } u;

    u.val = netshort;
    return ((uint16_t)u.raw[0] << 8) |
           ((uint16_t)u.raw[1]);
}
