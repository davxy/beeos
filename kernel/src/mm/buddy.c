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

#include "buddy.h"
#include "kprintf.h"
#include "kmalloc.h"
#include "util.h"
#include "panic.h"
#include <stddef.h>
#include <string.h>

/*
 * We use a bit for each couple of buddies.
 * This function toggles the bit corresponding to the couple
 * and returns the modified bit value.
 */
static int toggle_bit(const struct buddy_sys *ctx, unsigned int block_idx,
                      unsigned int order)
{
    unsigned int i;
    unsigned long *word, bit;

    i = block_idx >> (order + 1);
    word = &ctx->free_area[order].map[i / (8 * sizeof(unsigned long))];
    bit = 1UL << (i % (8 * sizeof(unsigned long)));
    *word ^= bit;           /* Toggle the bit value */
    return *word & bit;     /* Return the current value */
}

/*
 * Deallocate a frame
 */
void buddy_free(const struct buddy_sys *ctx, const struct frame *frm,
                unsigned int order)
{
    unsigned int block_idx, buddy_idx;

    block_idx = frm - ctx->frames;
    while (order != ctx->order_max) {
        /* Check if there is any buddy in the list of the same order */
        buddy_idx = block_idx ^ (1 << order);
        /*
         * Here we could have passed block_idx. Same bit would be toggled.
         * Non zero value is returned if the buddy is still allocated.
         */
        if (toggle_bit(ctx, buddy_idx, order) != 0)
            break;

        /* Remove the buddy from its free list */
        list_delete(&ctx->frames[buddy_idx].link);
        /* Coalesce into one bigger block */
        order++;

        /* Always keep track of the left-side index */
        if (buddy_idx < block_idx)
            block_idx = buddy_idx;
    }

    /* Insert the block at the end of the proper list */
    list_insert_before(&ctx->free_area[order].list,
            &ctx->frames[block_idx].link);
}

/*
 * Allocate a frame
 */
struct frame *buddy_alloc(const struct buddy_sys *ctx, unsigned int order)
{
    struct frame *frm = NULL;
    int left_idx, right_idx;
    unsigned int i;

    for (i = order; i <= ctx->order_max; i++) {
        if (!list_empty(&ctx->free_area[i].list)) {
            frm = list_container(ctx->free_area[i].list.next,
                                 struct frame, link);
            list_delete(&frm->link);
            left_idx = frm - ctx->frames;
            break;
        }
    }
    if (i > ctx->order_max)
        return NULL;

    if (i != ctx->order_max) /* Order max does't have any buddy */
        toggle_bit(ctx, left_idx, i);

    /* Eventually split */
    while (i > order) {
        i--;
        right_idx = left_idx + (1 << i);
        list_insert_before(&ctx->free_area[i].list,
                &ctx->frames[right_idx].link);
        toggle_bit(ctx, right_idx, i);
    }
    return frm;
}

/*
 * Initialize a buddy allocator
 */
int buddy_init(struct buddy_sys *ctx, unsigned int frames_num,
        unsigned int frame_size)
{
    unsigned int i;
    unsigned int count;
    unsigned int order_max;
    unsigned int order_bit;

    order_bit = fnzb(frame_size);
    order_max = fnzb(frames_num);
    if (order_bit + order_max >= 8 * sizeof(size_t))
        return -1;  /* Impossible value... */

    /*
     * Create the frames list
     */

    ctx->frames = (struct frame *)kmalloc(frames_num * sizeof(struct frame), 0);
    if (ctx->frames == NULL)
        goto e0;
    for (i = 0; i < frames_num; i++) {
        list_init(&ctx->frames[i].link);
        ctx->frames[i].refs = 1;
    }

    /*
     * Initialize the free frames table
     */

    ctx->order_bit = order_bit;
    ctx->order_max = order_max;
    ctx->free_area = (struct free_list *)kmalloc(sizeof(struct free_list) *
                                                (ctx->order_max+1), 0);
    if (ctx->free_area == NULL)
        goto e1;

    /*
     * Initialize free frames table row for each order.
     */

    for (i = 0; i < ctx->order_max; i++) {
        /* Num of buddies of order i. Divide number of blocks by 2^(i+1)  */
        count = (frames_num >> (i+1));
        /* Compute the required number of unsigned longs to hold the bitmap */
        count = (count - 1) / (8 * sizeof(unsigned long)) + 1;
        ctx->free_area[i].map =
                (unsigned long *)kmalloc(sizeof(unsigned long) * count, 0);
        if (ctx->free_area[i].map == NULL) {
            /* Rollback */
            while (i > 0) {
                kfree(ctx->free_area[i].map, sizeof(unsigned long) * count);
                count = (count + 1) / (8 * sizeof(unsigned long)) + 1;
            }
            goto e2;
        }
        memset(ctx->free_area[i].map, 0, sizeof(unsigned long) * count);
        list_init(&ctx->free_area[i].list);
    }

    /* Initialize the last (order_max) entry with a null buddy */
    list_init(&ctx->free_area[i].list);
    ctx->free_area[i].map = NULL;

    return 0;

    /* Rollback */
e2: kfree(ctx->free_area, sizeof(struct free_list) * (ctx->order_max+1));
e1: kfree(ctx->frames, frames_num * sizeof(struct frame));
e0: return -1;
}


/*
 * Dump buddy status
 */
void buddy_dump(const struct buddy_sys *ctx, char *base)
{
    unsigned int i;
    size_t freemem = 0;
    const struct list_link *frame_link;
    const struct frame *frm;
    unsigned int frame_idx;
    char *frame_ptr;

    kprintf("-----------------------------------------\n");
    kprintf("   Buddy Dump\n");
    kprintf("-----------------------------------------\n");
    for (i = 0; i <= ctx->order_max; i++) {
        kprintf("order: %d", i);
        if (list_empty(&ctx->free_area[i].list)) {
            kprintf("   [ empty ]\n");
        } else {
            kprintf("\n");
            for (frame_link = ctx->free_area[i].list.next;
                 frame_link != &ctx->free_area[i].list;
                 frame_link = frame_link->next) {
                frm = list_container_const(frame_link, struct frame, link);
                frame_idx = frm - ctx->frames;
                frame_ptr = base + (frame_idx << ctx->order_bit);
                kprintf("    [0x%p : 0x%p)\n", frame_ptr, frame_ptr +
                        (1 << (ctx->order_bit+i)));
                freemem += (1 << (ctx->order_bit + i));
            }
        }
    }
    kprintf("free: %u\n", freemem);
}
