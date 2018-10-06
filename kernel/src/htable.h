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

#ifndef BEEOS_HTABLE_H_
#define BEEOS_HTABLE_H_

#include <stdint.h>
#include <string.h>

/* https://gist.github.com/badboy/6267743 */

static inline unsigned int hash_32(uint32_t val, unsigned int bits)
{
    uint32_t v = val;

    v = (v ^ 61) ^ (v >> 16);
    v = v + (v << 3);
    v = v ^ (v >> 4);
    v = v * 0x27d4eb2d;   /* a prime or an odd constant */
    v = v ^ (v >> 15);
    /* High bits are more random, so use them */
    return v >> (32 - bits);
}

static inline unsigned int hash_64(uint64_t val, unsigned int bits)
{
    uint64_t v = val;

    v = (~v) + (v << 21);   /* v = (v << 21) - v - 1; */
    v = v ^ (v >> 24);
    v = (v + (v << 3)) + (v << 8);  /* v * 265 */
    v = v ^ (v >> 14);
    v = (v + (v << 2)) + (v << 4);  /* v * 21 */
    v = v ^ (v >> 28);
    v = v + (v << 31);
    return (unsigned int)(v >> (64 - bits));
}

#define hash(val, bits) \
    ((sizeof(val) <= 4) ? hash_32((val), (bits)) : hash_64((val), (bits)))

struct htable_link {
    struct htable_link *next;
    struct htable_link **pprev;
};

static inline void htable_init(struct htable_link **htable, unsigned int bits)
{
    memset(htable, 0, sizeof(struct htable_link *) * (1 << bits));
}

static inline void htable_insert(struct htable_link **htable,
                struct htable_link *node, long long key, unsigned int bits)
{
    int i;

    i = hash(key, bits);
    node->next = htable[i];
    node->pprev = &htable[i];
    if (htable[i] != NULL)
        htable[i]->pprev = (struct htable_link **)node;
    htable[i] = node;
}

static inline void htable_delete(const struct htable_link *node)
{
    struct htable_link *next = node->next;
    struct htable_link **pprev = node->pprev;

    *pprev = next;
    if (next != NULL)
        next->pprev = pprev;
}

static inline struct htable_link *htable_lookup(
        struct htable_link * const *htable,
        long long key, unsigned int bits)
{
    return htable[hash(key, bits)];
}

#endif /* BEEOS_HTABLE_H_ */
