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

#include "random.h"
#include "util.h"
#include "timer.h"
#include <stdint.h>


static uint32_t z[4];
static int rand_initialized = 0;

static uint32_t rand_get(void)
{
    uint32_t r;

    r = ((z[0] << 6) ^ z[0]) >> 13;
    z[0] = ((z[0] & 4294967294UL) << 18) ^ r;
    r = ((z[1] << 2) ^ z[1]) >> 27;
    z[1] = ((z[1] & 4294967288UL) << 2) ^ r;
    r = ((z[2] << 13) ^ z[2]) >> 21;
    z[2] = ((z[2] & 4294967280UL) << 7) ^ r;
    r = ((z[3] << 3) ^ z[3]) >> 12;
    z[3] = ((z[3] & 4294967168UL) << 13) ^ r;
    r = z[0] ^ z[1] ^ z[2] ^ z[3];
    return r;
}


int random_init(const unsigned char *seed, size_t seed_siz)
{
    int i = 0; /* Keeps track of dst */
    int j = 0; /* Keeps track of src */
    int k;
    unsigned char *dst = (unsigned char *)z;

    if (seed_siz == 0)
        return -1;

    for (k = 0; k < MAX(sizeof(z), seed_siz); k++) {
        dst[i] ^= seed[j];
        i++;
        if (i >= sizeof(z))
            i = 0;
        j++;
        if (j >= seed_siz)
            j = 0;
    }

    rand_initialized = 1;
    return 0;
}


static void rand_unaligned(unsigned char *buf, size_t n)
{
    uint32_t r;
    size_t i;

    r = rand_get();
    for (i = 0; i < n; i++) {
        *buf++ = (unsigned char)r;
        r >>= 8;
    }
}

ssize_t random_read(unsigned char *buf, size_t siz)
{
    size_t i;
    size_t iter;
    uint32_t *buf32;

    if (rand_initialized == 0) {
        if (random_init((const unsigned char *)&timer_ticks,
                        sizeof(timer_ticks)) < 0) {
            return -1;
        }
    }

    /* Eventually process front unaligned data */
    if (((iter = ((uintptr_t)buf & 0x03))) != 0)
        rand_unaligned(buf, sizeof(uint32_t) - iter);

    /* At this point, buf is be aligned to a uint32_t boundary (safe cast) */
    iter = (siz >> 2);
    buf32 = (uint32_t *)ALIGN_UP((uintptr_t)buf, sizeof(uint32_t));
    for (i = 0; i < iter; i++)
        buf32[i] = rand_get();

    /* Eventually process back unaligned data */
    if ((iter = (siz & 0x03)) != 0)
        rand_unaligned((unsigned char *)&buf32[i], iter);

    return (ssize_t)siz;
}
