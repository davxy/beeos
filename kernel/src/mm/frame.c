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

#include "frame.h"
#include "zone.h"
#include "kmalloc.h"
#include "kprintf.h"

/* List of all the registered zones */
static struct zone_st *zone_list;

void *frame_alloc(unsigned int order, int flags)
{
    void *ptr = NULL;
    struct zone_st *zone;
    
    for (zone = zone_list; zone != NULL; zone = zone->next) {
        if ((zone->flags & flags) != flags)
            continue;
        ptr = zone_alloc(zone, order);
        if (ptr)
            break;
    }
    return ptr;
}

void frame_free(void *ptr, unsigned int order)
{
    struct zone_st *zone;

    if (!ptr)
        return;
    for (zone = zone_list; zone != NULL; zone = zone->next) {
        if (iswithin((uintptr_t)zone->addr, zone->size,
                     (uintptr_t)ptr, 4096<<order)) {
            zone_free(zone, ptr, order);
            break;
        }
    }
}

int frame_zone_add(void *addr, size_t size, size_t frame_size, int flags)
{
    struct zone_st *zone;
    
    zone = kmalloc(sizeof(struct zone_st), 0);
    if (!zone)
        return -1;
    zone_init(zone, addr, size, frame_size, flags);
    zone->next = zone_list;
    zone_list = zone;
    return 0; 
}

void frame_dump(void)
{
    struct zone_st *zone;

    for (zone = zone_list; zone != NULL; zone = zone->next)
        zone_dump(zone);
}
