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

#include "kmalloc.h"
#include "mm/slab.h"
#include "util.h"

#define KMALLOC_MIN_W   4
#define KMALLOC_MAX_W   22


static int kmalloc_initialized = 0;

static struct slab_cache *kmalloc_caches[KMALLOC_MAX_W - KMALLOC_MIN_W + 1];

/*
 * Very primitive memory allocation form.
 * This is used silently used if the memory system has not been initialized.
 */
static void *ksbrk(intptr_t increment)
{
    void *ptr;
    extern char kend;        /* Defined in the linker script. */
    static char *brk = 0;
    if (!brk)
        brk = (char *)ALIGN_UP((uintptr_t)&kend, sizeof(uintptr_t));

    ptr = brk;
    brk += ALIGN_UP(increment, sizeof(uintptr_t));
    return ptr;
}

void *kmalloc(size_t size, int flags)
{
    unsigned int i;
    if (!kmalloc_initialized)
        return ksbrk(size);
    i = (size < 16) ? 16 : next_pow2(size);
    i >>= 4;
    i = fnzb(i);
    return slab_cache_alloc(kmalloc_caches[i], flags);
}

void kfree(void *ptr, size_t size)
{
    unsigned int i;
    if (!kmalloc_initialized)
        return;
    i= (size < 16) ? 16 : next_pow2(size);
    i >>= 4;
    i = fnzb(i);
    slab_cache_free(kmalloc_caches[i], ptr);
}

/* Initialize generic kernel memory allocator. */
void kmalloc_init(void)
{
    static const char *names[] = {
        "kmalloc-16", "kmalloc-32", "kmalloc-64", "kmalloc-128",
        "kmalloc-256", "kmalloc-512", "kmalloc-1K", "kmalloc-2K",
        "kmalloc-4K", "kmalloc-8K", "kmalloc-16K", "kmalloc-32K",
        "kmalloc-64K", "kmalloc-128K", "kmalloc-256K", "kmalloc-512K",
        "kmalloc-1M", "kmalloc-2M", "kmalloc-4M"
    };
    int i;
    size_t size;

    for (i = 0, size = (1 << KMALLOC_MIN_W);
         i < KMALLOC_MAX_W - KMALLOC_MIN_W + 1;
         i++, size <<= 1) {
        kmalloc_caches[i] = slab_cache_create(names[i], size, 0, 0, NULL, NULL);
    }
    kmalloc_initialized = 1;
}

