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

#ifndef BEEOS_MM_BUDDY_H_
#define BEEOS_MM_BUDDY_H_

#include "list.h"

/** Physical memory frame structure. */
struct frame {
    /** Free list link. */
    struct list_link    link;
    /** Number of references. */
    unsigned int        refs;
    /**
     * Context specific pointer.
     * E.g. if allocated by slab, this points there.
     */
    void                *ctx;
};

/** List of free frames with the same order. */
struct free_list {
    /** Free frames list node. */
    struct list_link    list;
    /** Bitmap used to keep track of the state of each couple of buddies. */
    unsigned long       *map;
};

/**
 * Buddy system context.
 * Allows to have a separate buddy allocator for each memory zone.
 */
struct buddy_sys {
    /** Log_2(frame_size). Frame size is passed to the init. */
    unsigned int        order_bit;
    /** Maximum frame order. Max frame size is 2^(order_bit+order_max). */
    unsigned int        order_max;
    /** Free frames list, one element for each order (order_max+1) */
    struct free_list    *free_area;
    /** Frames support structures (e.g. for the freelist) */
    struct frame        *frames;
};

/**
 * Initialize the buddy memory allocator context to handle a chunk of memory.
 *
 * @param ctx           Buddy system context pointer.
 * @param frames_num    Number of frames to be handled.
 * @param frame_size    Size of a single memory frame.
 * @return              Zero on success. A value less than zero on failure.
 */
int buddy_init(struct buddy_sys *ctx, unsigned int frames_num,
               unsigned int frame_size);

/**
 * Allocate a chunk of memory of the specified order.
 * The requested chunk size is 2^(order_bit + order).
 *
 * @param ctx       Buddy system context pointer.
 * @param order     Requested chunk order.
 * @return          Memory chunk start frame.
 */
struct frame *buddy_alloc(const struct buddy_sys *ctx, unsigned int order);

/**
 * Release a chunk of memory.
 *
 * @param ctx       Buddy system context pointer.
 * @param frame     Memory chunk start frame.
 * @param order     Memory chunk order.
 */
void buddy_free(const struct buddy_sys *ctx, const struct frame *frm,
                unsigned int order);

/**
 * Prints buddy system status.
 *
 * @param ctx       Buddy system context pointer.
 * @param base      Memory start
 */
void buddy_dump(const struct buddy_sys *ctx, char *base);

#endif /* BEEOS_MM_BUDDY_H_ */
