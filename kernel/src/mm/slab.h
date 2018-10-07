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

#ifndef BEEOS_MM_SLAB_H_
#define BEEOS_MM_SLAB_H_

#include "list.h"
#include <sys/types.h>  /* size_t */

typedef void (* slab_obj_ctor_t)(void *obj);
typedef void (* slab_obj_dtor_t)(void *obj);

/** Slab cache structure */
struct slab_cache {
    const char          *name;          /**< Cache name string  */
    unsigned int        flags;          /**< Cache flags */
    size_t              objsize;        /**< Single object size */
    unsigned int        slab_objs;      /**< Objects per slab */
    struct list_link    slabs_full;     /**< List of full slabs */
    struct list_link    slabs_part;     /**< List of partial slabs */
    slab_obj_ctor_t     ctor;           /**< Object constructor */
    slab_obj_dtor_t     dtor;           /**< Object destructor */
    struct htable_link  **htable;       /**< Hash table */
    size_t              hload;          /**< Hash table load */
    size_t              hsize;          /**< Hash table size */
};

void slab_init(void);

struct slab_cache *slab_cache_create(const char *name,
        size_t size, unsigned int align, unsigned int flags,
        slab_obj_ctor_t ctor, slab_obj_dtor_t dtor);

void slab_cache_delete(struct slab_cache *cache);

void slab_cache_init(struct slab_cache *cache, const char *name,
        size_t objsize, unsigned int align, unsigned int flags,
        slab_obj_ctor_t ctor, slab_obj_dtor_t dtor);

void slab_cache_deinit(struct slab_cache *cache);

void *slab_cache_alloc(struct slab_cache *cache, int flags);

void slab_cache_free(struct slab_cache *cache, void *obj);


#endif /* BEEOS_MM_SLAB_H_ */
