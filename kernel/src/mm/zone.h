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

#ifndef BEEOS_MM_ZONE_H_
#define BEEOS_MM_ZONE_H_

#include "buddy.h"
#include "list.h"

/**
 * Low memory is used to allocate buffers that must be identity mapped
 * to physical memory. That is we can pass from the physical address to
 * the virtual address by adding the kernel virtual address base.
 * The user should use the virt_to_phys and phys_to_virt functions.
 */
#define ZONE_HIGH   0
#define ZONE_LOW    1

/** Zone descriptor */
struct zone_st {
    char            *addr;       /**< Zone (physical) address */
    size_t           size;       /**< Zone size */
    size_t           frame_size; /**< Size of a single frame */
    size_t           free_count; /**< Number of free frames */
    size_t           busy_count; /**< Number of busy frames */
    unsigned char    flags;      /**< Type of the zone (e.g. ZONE_HIGH) */
    struct frame_st *frames;     /**< Array of frame structures in this zone */
    struct zone_st  *next;       /**< Link to next zone */
    struct buddy_sys buddy;      /**< Buddy system for the zone */
};

/**
 * Initialize a zone descriptor structure.
 *
 * @param ctx           Zone descriptor structure.
 * @param addr          Zone physical address.
 * @param size          Zone size.
 * @param frame_size    Size of the frame within the zone.
 * @param flags         Zone flags (e.g. ZONE_HIGH).
 * @return              On error -1 is returned.
 */
int zone_init(struct zone_st *ctx, void *addr, size_t size,
              size_t frame_size, int flags);

/**
 * Allocate a memory segment from a zone.
 * The allocation happens by powers of two of the frame size.
 * That is the memory chunk returned has size equal to frame_size * 2^order.
 *
 * @param ctx   Zone descriptror structure.
 * @param order Frame order.
 * @return      Pointer to the allocated memory chunk.
 */
void *zone_alloc(const struct zone_st *ctx, int order);

/**
 * Free a memory segment from the zone.
 *
 * @param ctx   Zone descriptor structure.
 * @param ptr   Pointer to the memory chunk
 * @param order Frame order.
 */
void zone_free(const struct zone_st *ctx, const void *ptr, int order);

/**
 * DEBUG function.
 * Dumps the current memory situation on the stdout.
 *
 * @param ctx   Zone descriptor structure.
 */
void zone_dump(const struct zone_st *ctx);

#endif /* BEEOS_MM_ZONE_H_ */
