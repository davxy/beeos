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

#include "kmalloc.h"
#include "mm/slab.h"
#include "util.h"


#define KMALLOCS_SLABS_NUM  19

static struct slab_cache *kmalloc_caches[KMALLOCS_SLABS_NUM];

static const char *names[KMALLOCS_SLABS_NUM] = {
    "kmalloc-16",
    "kmalloc-32",
    "kmalloc-64",
    "kmalloc-128",
    "kmalloc-256",
    "kmalloc-512",
    "kmalloc-1K",
    "kmalloc-2K",
    "kmalloc-4K",
    "kmalloc-8K",
    "kmalloc-16K",
    "kmalloc-32K",
    "kmalloc-64K",
    "kmalloc-128K",
    "kmalloc-256K",
    "kmalloc-512K",
    "kmalloc-1M",
    "kmalloc-2M",
    "kmalloc-4M"
};

static int kmalloc_initialized = 0;

/*
 * Align to the next power of two
 */
static unsigned long next_pow2(unsigned long val)
{
    unsigned long v = val;

    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

/*
 * Very primitive memory allocation form.
 * This is used silently used if the memory system has not been initialized.
 */
static void *ksbrk(intptr_t increment)
{
    void *ptr;
    extern char kend;        /* Defined in the linker script. */
    static char *kbrk = NULL;

    if (kbrk == NULL)
        kbrk = (char *)ALIGN_UP((uintptr_t)&kend, sizeof(uintptr_t));
    ptr = kbrk;
    kbrk += ALIGN_UP(increment, sizeof(uintptr_t));
    return ptr;
}

void *kmalloc(size_t size, int flags)
{
    unsigned int i;

    if (kmalloc_initialized == 0)
        return ksbrk(size);
    i = (size < 16) ? 16 : next_pow2(size);
    i >>= 4;
    i = fnzb(i);
    return slab_cache_alloc(kmalloc_caches[i], flags);
}

void kfree(void *ptr, size_t size)
{
    unsigned int i;

    if (kmalloc_initialized == 0)
        return;
    i= (size < 16) ? 16 : next_pow2(size);
    i >>= 4;
    i = fnzb(i);
    slab_cache_free(kmalloc_caches[i], ptr);
}


/* Initialize generic kernel memory allocator. */
void kmalloc_init(void)
{
    int i;
    size_t size;

    size = 16; /* Min slab size */
    for (i = 0; i < KMALLOCS_SLABS_NUM; i++) {
        kmalloc_caches[i] = slab_cache_create(names[i], size, 0, 0, NULL, NULL);
        size <<= 1;
    }
    kmalloc_initialized = 1;
}
