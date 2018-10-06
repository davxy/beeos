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

#ifndef BEEOS_MM_FRAME_H_
#define BEEOS_MM_FRAME_H_

#include "mm/zone.h"
#include <sys/types.h>

/**
 * Allocate a physical memory page.
 *
 * @param order Frame order.
 * @param flags Allocation flags.
 * @return      Memory physical address.
 */
void *frame_alloc(unsigned int order, unsigned int flags);

/**
 * Free a physical memory page.
 *
 * @param ptr   Memory physical address.
 * @param order Frame order.
 */
void frame_free(void *ptr, unsigned int order);

/**
 * Add a memory zone to the frame allocator.
 *
 * @param addr          Zone frame address.
 * @param size          Size of the zone.
 * @param frame_size    Size of frames within this zone.
 * @param flags         Frame zone flags.
 * @return              0 on success, -1 on error.
 */
int frame_zone_add(void *addr, size_t size, size_t frame_size, int flags);

/**
 * Frame allocator dump function.
 */
void frame_dump(void);


#endif /* BEEOS_MM_FRAME_H_ */
