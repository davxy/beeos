/*
 * Copyright (c) 2015-2017, Davide Galassi. All rights reserved.
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

#ifndef BEEOS_HTABLE_H_
#define BEEOS_HTABLE_H_

#include <stdint.h>
#include <string.h>

/* https://gist.github.com/badboy/6267743 */

static inline int hash_32(uint32_t val, int bits)
{
    val = (val ^ 61) ^ (val >> 16);
    val = val + (val << 3);
    val = val ^ (val >> 4);
    val = val * 0x27d4eb2d;   /* a prime or an odd constant */
    val = val ^ (val >> 15);
    /* High bits are more random, so use them */
    return val >> (32 - bits);
}

static inline int hash_64(uint64_t val, int bits)
{
    val = (~val) + (val << 21); // val = (val << 21) - val - 1;
    val = val ^ (val >> 24);
    val = (val + (val << 3)) + (val << 8); // val * 265
    val = val ^ (val >> 14);
    val = (val + (val << 2)) + (val << 4); // val * 21
    val = val ^ (val >> 28);
    val = val + (val << 31);
    return (int)(val >> (64 - bits));
}

#define hash(val, bits) \
    ((sizeof(val) <= 4) ? hash_32((val), (bits)) : hash_64((val), (bits)))

struct htable_link
{
    struct htable_link *next;
    struct htable_link **pprev;
};

static inline void htable_init(struct htable_link **htable, int bits)
{
    memset(htable, 0, sizeof(struct htable_link *)*(1 << bits));
}

static inline void htable_insert(struct htable_link **htable,
        struct htable_link *node, long long key, int bits)
{
    int i;

    i = hash(key, bits);
    node->next = htable[i];
    node->pprev = &htable[i];
    if (htable[i])
        htable[i]->pprev = (struct htable_link **)node;
    htable[i] = node;
}

static inline void htable_delete(const struct htable_link *node)
{
    struct htable_link *next = node->next;
    struct htable_link **pprev = node->pprev;

    *pprev = next;
    if (next)
        next->pprev = pprev;
}

static inline struct htable_link *htable_lookup(
        struct htable_link * const *htable,
        long long key, int bits)
{
    return htable[hash(key, bits)];
}

#endif /* BEEOS_HTABLE_H_ */

