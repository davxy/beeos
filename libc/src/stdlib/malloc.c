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

/*
 * Simple K&R allocator described by the "C programming language" book.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

struct malloc_head {
    struct malloc_head *next;   /* Next block if on free list */
    size_t size;                /* Size of the block */
};

static struct malloc_head base;            /* empty list to get started */
static struct malloc_head *freep = NULL;   /* start of free list */

#define ALIGN           sizeof(void *)
#define ALIGN_UP(val)   (((val) + ((ALIGN) - 1)) & ~((ALIGN) - 1))
#define ALIGN_DOWN(val) ((val) & ~((ALIGN) - 1))
#define TO_HEAD(ptr) \
    (struct malloc_head *)ALIGN_DOWN((uintptr_t)ptr - sizeof(struct malloc_head))
#define TO_DATA(ptr) \
    (void *)ALIGN_UP((uintptr_t)ptr + sizeof(struct malloc_head))

#define NALLOC  (1024*ALIGN)


void free(void *ptr)
{
    struct malloc_head *curr, *prev;

    curr = TO_HEAD(ptr);
    for (prev = freep; !(prev < curr && curr < prev->next);
         prev = prev->next) {
        if (prev >= prev->next && (curr > prev || curr < prev->next))
            break;  /* freed a block at start or end */
    }

    if ((char *)curr + curr->size == (char *)prev->next) {
        /* join to the upper */
        curr->size += prev->next->size;
        curr->next = prev->next->next;
    } else {
        curr->next =prev->next;
    }

    if ((char *)prev + prev->size == (char *)curr) {
        prev->size += curr->size;
        prev->next = curr->next;
    } else {
        prev->next = curr;
    }

    freep = prev;
}

static struct malloc_head *morecore(size_t size)
{
    struct malloc_head *p;

    if (size < NALLOC)
        size = NALLOC;

    p = (struct malloc_head *)sbrk(size);
    if (p == (struct malloc_head *)-1) {
        errno = ENOMEM;
        return NULL;
    }
    p->size = size;
    free(TO_DATA(p));
    return freep;
}

void *malloc(size_t size)
{
    struct malloc_head *curr, *prev;

    /* size adjust */
    size = ALIGN_UP(size + sizeof(struct malloc_head));

    if ((prev = freep) == NULL) {
        /* first invocation */
        base.next = freep = prev = &base;
        base.size = 0;
    }

    for (curr = freep->next; ; prev = curr, curr = curr->next) {
        if (curr->size >= size) {
            if (curr->size == size) {
                prev->next = curr->next;
            } else {
                /* allocate tail end */
                curr->size -= size;
                curr = (struct malloc_head *)((char *)curr + curr->size);
                curr->size = size;
            }
            freep = prev;
            return TO_DATA(curr);
        }
        if (curr == freep) {
            /* frapped around free list */
            if ((curr = morecore(size)) == NULL)
                return NULL;    /* none left */
        }
    }
}


void *realloc(void *ptr, size_t size)
{
    void *new_ptr;
    struct malloc_head *head;
    size_t old_size;

    new_ptr = malloc(size);
    if (new_ptr == NULL || ptr == NULL)
        return new_ptr;

    head = TO_HEAD(ptr);
    old_size = head->size - ((char *)ptr - (char *)head);

    if (old_size < size)
        memcpy(new_ptr, ptr, old_size);
    else
        memcpy(new_ptr, ptr, size);
    free(ptr);
    return new_ptr;
}

void *calloc(size_t nmemb, size_t size)
{
    void *ptr;
    size_t totsiz;

    totsiz = nmemb * size;
    ptr = malloc(totsiz);
    if (ptr != NULL)
        memset(ptr, 0, totsiz);
    return ptr;
}
