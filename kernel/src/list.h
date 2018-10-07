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

#ifndef BEEOS_LIST_H_
#define BEEOS_LIST_H_

#include <stddef.h>     /* offsetof */
#include "util.h"


/** Circularly double linked list data structure. */
struct list_link {
    /** Previous element. */
    struct list_link    *prev;
    /** Next element. */
    struct list_link    *next;
};

/**
 * Initialize a list.
 * The link is made point to itself.
 *
 * @param link  List link pointer.
 */
static inline void list_init(struct list_link *list)
{
    list->next = list;
    list->prev = list;
}

static inline void list_insert_after(struct list_link *list,
                                     struct list_link *node)
{
    node->next = list->next;
    node->prev = list;
    list->next->prev = node;
    list->next = node;
}

static inline void list_insert_before(struct list_link *list,
                                      struct list_link *node)
{
    node->next = list;
    node->prev = list->prev;
    list->prev->next = node;
    list->prev = node;
}

/**
 * Unlink an entry from its list.
 * After unlinked the element is a single element list.
 *
 * @param link  The element to delete from the list.
 */
static inline void list_delete(struct list_link *link)
{
    link->next->prev = link->prev;
    link->prev->next = link->next;
    link->next = link;
    link->prev = link;
}

/**
 * Merges together two initialized lists.
 *
 * @param list1
 */
static inline void list_merge(struct list_link *list1,
                              struct list_link *list2)
{
    struct list_link *tmp;
    list1->prev->next = list2;
    list2->prev->next = list1;
    tmp = list1->prev;
    list1->prev = list2->prev;
    list2->prev = tmp;
}

/**
 * Check if the list is empty.
 *
 * @param link  List link pointer.
 * @return      True if it is empty.
 */
#define list_empty(link) ((link)->next == (link))

/**
 * Get a pointer to the struct start for this list element.
 *
 * @elem:   the struct list_link pointer.
 * @type:   the type of the struct the element is embedded in.
 * @member: the name of the list_link within the struct.
 */
#define list_container(link, type, member) \
    ((type *) ((char *)(link) - offsetof(type, member)))

/**
 * Get a pointer to the struct start for this list element.
 * Constant version.
 *
 * @elem:   the struct list_link pointer.
 * @type:   the type of the struct the element is embedded in.
 * @member: the name of the list_link within the struct.
 */
#define list_container_const(link, type, member) \
    ((const type *) ((const char *)(link) - offsetof(const type, member)))


#endif /* BEEOS_LIST_H_ */
