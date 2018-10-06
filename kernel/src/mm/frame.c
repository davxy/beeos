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

#include "frame.h"
#include "zone.h"
#include "kmalloc.h"
#include "kprintf.h"


/* List of all the registered zones */
static struct zone_st *zone_list;

void *frame_alloc(unsigned int order, unsigned int flags)
{
    void *ptr = NULL;
    const struct zone_st *zone;

    for (zone = zone_list; zone != NULL; zone = zone->next) {
        if ((zone->flags & flags) == flags) {
            ptr = zone_alloc(zone, order);
            if (ptr != NULL)
                break;
        }
    }
    return ptr;
}


static int iswithin(uintptr_t b1, size_t sz1, uintptr_t b2, size_t sz2)
{
    uintptr_t e1;
    uintptr_t e2;

    if (sz1 == 0)
        return ((b1 == b2) && (sz2 == 0));
    e1 = b1 + sz1 - 1;
    if (sz2 == 0)
        return ((b1 <= b2) && (b2 <= e1));
    e2 = b2 + sz2 - 1;
    /* e1 and e2 are end addresses, the sum is immune to overflow */
    return ((b1 <= b2) && (e1 >= e2));
}


void frame_free(void *ptr, unsigned int order)
{
    const struct zone_st *zone;

    if (ptr == NULL)
        return;
    for (zone = zone_list; zone != NULL; zone = zone->next) {
        if (order <= zone->buddy.order_max &&
            iswithin((uintptr_t)zone->addr, zone->size, (uintptr_t)ptr,
                     (size_t)1 << (order + zone->buddy.order_bit)) != 0) {
            zone_free(zone, ptr, order);
            break;
        }
    }
}

int frame_zone_add(void *addr, size_t size, size_t frame_size, int flags)
{
    int res;
    struct zone_st *zone;

    zone = (struct zone_st *)kmalloc(sizeof(struct zone_st), 0);
    if (zone != NULL) {
        res = zone_init(zone, addr, size, frame_size, flags);
        if (res == 0) {
            zone->next = zone_list;
            zone_list = zone;
        } else {
            kfree(zone, sizeof(struct zone_st));
        }
    } else {
        res = -1;
    }
    return res;
}


void frame_dump(void)
{
    const struct zone_st *zone;

    for (zone = zone_list; zone != NULL; zone = zone->next)
        zone_dump(zone);
}
