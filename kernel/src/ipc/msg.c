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

#include "ipc/msg.h"
#include "htable.h"

#define HTABLE_BITS 3
struct htable_link *ipc_hmap[1<<HTABLE_BITS];
int ipc_hmap_initialized;

int msgget(key_t key, int flags)
{
    struct msqid_ds *msgq;

    if (!ipc_hmap_initialized)
        htable_init(&ipc_hmap, HTABLE_BITS);

    msgq = htable_lookup(&ipc_hmap, key, HTABLE_BITS);
    if (!msgq)
    {
        if ((msgq = kmalloc(sizeof(*msgq), 0)) == NULL)
            return -1;
        htable_insert(&ipc_hmap, &msgq.hnode, key, HTABLE_BITS);
    }
    return 0;
}
