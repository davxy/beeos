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

#ifndef BEEOS_UTIL_H_
#define BEEOS_UTIL_H_

#include <stdint.h>
#include <sys/types.h>


#define ALIGN_UP(val, a) (((val) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(val, a) ((val) & ~((a) - 1))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/**
 * First non zero bit position starting from left.
 *
 * @param v     Value under analysis.
 * @return      Zero based bit position.
 *              If the input value is zero then returns 0.
 */
static inline unsigned int fnzb(unsigned long val)
{
    unsigned int n = 0;
    unsigned long v = val;

    if ((v >> 16) != 0) {
        v >>= 16;
        n += 16;
    }
    if ((v >> 8) != 0) {
        v >>= 8;
        n += 8;
    }
    if ((v >> 4) != 0) {
        v >>= 4;
        n += 4;
    }
    if ((v >> 2) != 0) {
        v >>= 2;
        n += 2;
    }
    if ((v >> 1) != 0) {
        v >>= 1;
        n += 1;
    }
    return n;
}

/**
 * Get a pointer to the struct start given a pointer to a member.
 *
 * @param member_ptr    Struct member pointer.
 * @param struct_type   Type of the structure the element is embedded in.
 * @param member_name   Name of the member within the struct.
 */
#define struct_ptr(member_ptr, struct_type, member_name) \
    ((struct_type *)((char *)(member_ptr)-offsetof(struct_type,member_name)))


#endif /* BEEOS_UTIL_H_ */
