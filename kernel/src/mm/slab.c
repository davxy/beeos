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

#include "mm/frame.h"
#include "mm/slab.h"
#include "kmalloc.h"
#include "util.h"
#include "panic.h"
#include "htable.h"
#include "kprintf.h"
#include <stdint.h>
#include <string.h>
#include "arch/x86/vmem.h"
#include "arch/x86/paging_bits.h"


/* Memory alignment */
#define ALIGN_VALUE         sizeof(void *)

#define SLAB_UNIT_BITS      12
#define SLAB_UNIT_SIZE      PAGE_SIZE   /* MUST be a multiple of page size */

/* Small slabs size limit */
#define SLAB_SMALL_MAX      (SLAB_UNIT_SIZE >> 3)

#define SLABCTL_OFFSET      (SLAB_UNIT_SIZE-sizeof(struct slabctl))

#define BUF_TO_SLABCTL(buf) \
    ((struct slabctl *) (((uintptr_t)(buf) & ~(SLAB_UNIT_SIZE-1)) + SLABCTL_OFFSET))

#define BUF_TO_BUFCTL(buf, objsz) \
    ((struct bufctl *) ((char *)(buf) + (objsz) - sizeof(struct bufctl *)))

#define BUFCTL_TO_BUF(bctl, objsz) \
    ((char *)(bctl) + sizeof(struct bufctl *) - (objsz))

/* Flags */
#define SLAB_EMBED_BUFCTL       0x01    /* bufctl is at buf end */
#define SLAB_EMBED_SLABCTL      0x02    /* slabctl is at slab end */
#define SLAB_OPTIMIZE           0x04    /* Optimize slab allocation */

/*
 * The bufctl (buffer control) structure keeps some minimal information
 * about each buffer: its address, its slab, and its current linkage,
 * which is either on the slab's freelist (if the buffer is free), or
 * on the cache's buf-to-bufctl hash table (if the buffer is allocated).
 * In the case of non-hashed, or "raw", caches (the common case), only
 * the freelist linkage is necessary: the buffer address is at a fixed
 * offset from the bufctl address, and the slab is at the end of the page.
 *
 * NOTE: bc_next must be the first field; small buffers have linkage only.
 */
struct bufctl {
    struct htable_link  hlink;  /* Next buffer control structure */
    struct slabctl      *slab;  /* Buffer slab */
    void                *buf;   /* Buffer start address */
};

struct slabctl {
    unsigned int        inuse;  /* Entries in use */
    struct list_link    link;   /* Full, partial, free list link */
    void                *data;  /* Address of the first available item */
    struct htable_link  *bctls; /* List of free bufctls (exploiting hlist) */
    struct slab_cache   *cache; /* Slab cache pointer */
};


/* Cache for caches. Pre-allocated to prevent the chicken and egg problem. */
static struct slab_cache slab_cache_cache;
/* Cache for external slab control data */
static struct slab_cache *slab_slabctl_cache;
/* Cache for external buffer control data */
static struct slab_cache *slab_bufctl_cache;


static void *bufctl_hash_put(struct slab_cache *cache, struct bufctl *bctl)
{
    if (cache->hload == 0) {
        cache->hsize = 32;
        cache->htable = (struct htable_link  **)kmalloc(cache->hsize *
                                sizeof(struct htable_link *), 0);
        if (cache->htable == NULL)
            return NULL;
        htable_init(cache->htable, fnzb(cache->hsize));
    }

    htable_insert(cache->htable, &bctl->hlink, (uintptr_t)bctl->buf,
            fnzb(cache->hsize));
    if (bctl->hlink.next == NULL)
        cache->hload++; /* added to an empty slot */
    return bctl->buf;
}

static struct bufctl *bufctl_hash_get(struct slab_cache *cache, void *obj)
{
    struct htable_link *link;
    struct bufctl *bctl;
    size_t size;

    if (cache->htable == NULL)
        return NULL;

    link = htable_lookup(cache->htable, (uintptr_t)obj,
            fnzb(cache->hsize));

    /* find the correct link */
    while (link != NULL) {
        bctl = struct_ptr(link, struct bufctl, hlink);
        if (bctl->buf == obj)
            break;
        link = link->next;
    }
    if (link == NULL)
        return NULL;

    htable_delete(link);
    if (cache->hload == 0) {
        size = cache->hsize * sizeof(struct htable_link *);
        kfree(cache->htable, size);
        cache->htable = NULL;
        cache->hsize = 0;
    }
    return bctl;
}

/*
 * Simple linked list holding available bufctl structures.
 * The list is created exploiting the hash list nodes.
 */

static struct bufctl *bufctl_list_get(struct slabctl *slab)
{
    struct htable_link *lnk;

    if (slab->bctls == NULL)
        return NULL;
    lnk = slab->bctls;
    slab->bctls = lnk->next;
    slab->inuse++;
    return struct_ptr(lnk, struct bufctl, hlink);
}

static void bufctl_list_put(struct slabctl *slab, struct bufctl *bctl)
{
    slab->inuse--;
    bctl->hlink.next = slab->bctls;
    slab->bctls = &bctl->hlink;
}

static void slab_space_free(struct slabctl* slab, size_t size)
{
    int i;
    const struct slab_cache *cache = slab->cache;
    void *data = slab->data;
    void *obj;
    unsigned int order;

    obj = data;
    for (i = 0; i < cache->slab_objs; i++) {
        if (cache->dtor != NULL)
            cache->dtor(obj);

        if ((cache->flags & SLAB_EMBED_BUFCTL) == 0) {
            struct bufctl *bctl = bufctl_list_get(slab);
            slab_cache_free(slab_bufctl_cache, bctl);
        }
        obj = (char *)obj + cache->objsize;
    }

    if ((cache->flags & SLAB_EMBED_SLABCTL) == 0)
        slab_cache_free(slab_slabctl_cache, slab);

    order = fnzb(size >> SLAB_UNIT_BITS);
    if ((1 << (order + SLAB_UNIT_BITS)) < size)
        order++;
    frame_free(virt_to_phys(data), order);
}

static struct slabctl *slab_space_alloc(struct slab_cache *cache, int flags)
{
    int i;
    void *obj;
    struct slabctl *slab;
    struct bufctl *bctl;
    size_t size;
    void *data;
    unsigned int order;
    unsigned int objs;

    size = ALIGN_UP(cache->slab_objs * cache->objsize, SLAB_UNIT_SIZE);
    order = fnzb(size >> SLAB_UNIT_BITS);
    if ((1 << (order + SLAB_UNIT_BITS)) < size)
        order++;
    data = frame_alloc(order, 0);
    if (data == NULL)
        return NULL;
    data = phys_to_virt(data);

    if ((cache->flags & SLAB_EMBED_SLABCTL) != 0) {
        slab = BUF_TO_SLABCTL(data);
    } else {
        slab = (struct slabctl *)slab_cache_alloc(slab_slabctl_cache, flags);
        if (slab == NULL) {
            frame_free(virt_to_phys(data), order);
            return NULL;
        }
    }

    slab->data = data;
    slab->inuse = cache->slab_objs; /* released by bufctl_list_put */
    slab->cache = cache;
    slab->bctls = NULL;
    list_init(&slab->link);

    obj = data;
    for (i = 0; i < cache->slab_objs; i++) {
        if ((cache->flags & SLAB_EMBED_BUFCTL) != 0) {
            bctl = BUF_TO_BUFCTL(obj, cache->objsize);
        } else {
            bctl = (struct bufctl *)slab_cache_alloc(slab_bufctl_cache, flags);
            if (bctl == NULL) {
                /* temporary set to the allocated objects and undo */
                objs = slab->cache->slab_objs;
                slab->cache->slab_objs = i;
                slab_space_free(slab, size);
                slab->cache->slab_objs = objs;
                return NULL;
            }
            bctl->buf = obj;
            bctl->slab = slab;
        }
        bufctl_list_put(slab, bctl);
        if (cache->ctor != NULL)
            cache->ctor(obj);

        obj = (char *)obj + cache->objsize;
    }
    return slab;
}


void *slab_cache_alloc(struct slab_cache *cache, int flags)
{
    void *obj = NULL;
    struct slabctl *slab;
    struct bufctl *bctl;

    if (list_empty(&cache->slabs_part) == 0) {
        slab = list_container(cache->slabs_part.next, struct slabctl, link);
        list_delete(&slab->link);
    } else {
        slab = slab_space_alloc(cache, flags);
        if (slab == NULL)
            return NULL;
    }

    bctl = bufctl_list_get(slab);
    if ((cache->flags & SLAB_EMBED_BUFCTL) != 0) {
        obj = BUFCTL_TO_BUF(bctl, cache->objsize);
    } else {
        obj = bufctl_hash_put(cache, bctl);
        if (obj == NULL) {
            bufctl_list_put(slab, bctl);
            return NULL;
        }
    }

    if ((cache->slab_objs - slab->inuse) > 0)
        list_insert_after(&cache->slabs_part, &slab->link);
    else
        list_insert_after(&cache->slabs_full, &slab->link);

    return obj;
}

void slab_cache_free(struct slab_cache *cache, void *obj)
{
    struct slabctl *slab;
    struct bufctl *bctl;
    size_t size;

    size = ALIGN_UP(cache->slab_objs * cache->objsize, SLAB_UNIT_SIZE);

    if ((cache->flags & SLAB_EMBED_BUFCTL) != 0) {
        bctl = BUF_TO_BUFCTL(obj, cache->objsize);
        slab = BUF_TO_SLABCTL(obj);
    } else {
        bctl = bufctl_hash_get(cache, obj);
        if (bctl == NULL)
            return;
        slab = bctl->slab;
    }
    bufctl_list_put(slab, bctl);

    if (slab->inuse == 0) {
        list_delete(&slab->link);
        slab_space_free(slab, size);
    } else if (slab->inuse == cache->slab_objs - 1) {
        list_delete(&slab->link);
        list_insert_after(&cache->slabs_part, &slab->link);
    }
}


void slab_cache_init(struct slab_cache *cache, const char *name,
        size_t objsize, unsigned int align, unsigned int flags,
        slab_obj_ctor_t ctor, slab_obj_dtor_t dtor)
{
    size_t slabsize, wasted, orgsiz;

    align = (align < ALIGN_VALUE) ?
        ALIGN_VALUE : ALIGN_UP(align, ALIGN_VALUE);

    memset(cache, 0, sizeof(struct slab_cache));
    cache->name = name;
    cache->objsize = ALIGN_UP(objsize, align);
    cache->ctor = ctor;
    cache->dtor = dtor;
    cache->flags = flags;

    list_init(&cache->slabs_full);
    list_init(&cache->slabs_part);

    cache->htable = NULL;
    cache->hsize = 0;
    cache->hload = 0;

    if (cache->objsize <= SLAB_SMALL_MAX) {
        if (ctor == NULL) {
            cache->flags |= (SLAB_EMBED_BUFCTL | SLAB_EMBED_SLABCTL);
        } else {
            /* Add the space for bufctl link */
            cache->objsize += sizeof(struct bufctl *);
            if (cache->objsize <= SLAB_SMALL_MAX)
                cache->flags |= (SLAB_EMBED_BUFCTL | SLAB_EMBED_SLABCTL);
            else
                cache->objsize -= sizeof(struct bufctl *);
        }
    }

    slabsize = ALIGN_UP(cache->objsize, SLAB_UNIT_SIZE);
    if ((cache->flags & SLAB_EMBED_SLABCTL) != 0)
        slabsize -= sizeof(struct slabctl);

    /*
     * NOTE
     * If objsize is less than (or equal) SLAB_SMALL_MAX = PAGE_SIZE/8
     * then the wasted space is always less than slabsize/4.
     * slabsize % SMALL_MAX < SMALL_MAX = PAGESIZE/8 < slabsize/4
     */
    if ((flags & SLAB_OPTIMIZE) != 0) {
        orgsiz = slabsize;
        do {
            cache->slab_objs = slabsize / cache->objsize;
            wasted = slabsize % cache->objsize;
            /* wasted space should be less than slabsize/4 */
            if (wasted <= (slabsize >> 2)) {
                if (sizeof(struct slabctl) <= wasted)
                    cache->flags |= SLAB_EMBED_SLABCTL;
                break;
            }
            slabsize <<= 1;
        } while(slabsize > orgsiz);

        if (slabsize <= orgsiz) {
            /* Should never happen, but be defensive... */
            cache->slab_objs = slabsize / cache->objsize;
        }
    } else {
        cache->slab_objs = slabsize / cache->objsize;
    }
}

void slab_cache_deinit(struct slab_cache *cache)
{
    struct slabctl *slab;
    size_t size;

    size = ALIGN_UP(cache->slab_objs*cache->objsize, SLAB_UNIT_SIZE);
    while (list_empty(&cache->slabs_part) == 0) {
        slab = list_container(cache->slabs_part.next, struct slabctl, link);
        list_delete(&slab->link);
        slab_space_free(slab, size);
    }
    while (list_empty(&cache->slabs_full) == 0) {
        slab = list_container(cache->slabs_part.next, struct slabctl, link);
        list_delete(&slab->link);
        slab_space_free(slab, size);
    }
    memset(cache, 0, sizeof(struct slab_cache));
}

struct slab_cache *slab_cache_create(const char *name,
        size_t size, unsigned int align, unsigned int flags,
        slab_obj_ctor_t ctor, slab_obj_dtor_t dtor)
{
    struct slab_cache *cache;

    cache = (struct slab_cache *)slab_cache_alloc(&slab_cache_cache, 0);
    if (cache != NULL)
        slab_cache_init(cache, name, size, align, flags, ctor, dtor);
    return cache;
}

void slab_cache_delete(struct slab_cache *cache)
{
    slab_cache_deinit(cache);
    slab_cache_free(&slab_cache_cache, cache);
}

void slab_init(void)
{
    /* Initialize the caches cache */
    slab_cache_init(&slab_cache_cache, "slab_cache_cache",
            sizeof(slab_cache_cache), sizeof(void *), 0,
            NULL, NULL);

    /* Create a cache for slabs */
    slab_slabctl_cache = slab_cache_create("slab_slabctl_cache",
            sizeof(struct slabctl), 0, 0, NULL, NULL);
    if (slab_slabctl_cache == NULL)
        panic("slab_slabctl_cache creation error");

    /* Create a cache for bufctl */
    slab_bufctl_cache = slab_cache_create("slab_bufctl_cache",
            sizeof(struct bufctl), 0, 0, NULL, NULL);
    if (slab_bufctl_cache == NULL)
        panic("slab_bufctl_cache creation error");
}
