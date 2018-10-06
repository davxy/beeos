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

#ifndef BEEOS_SYNC_COND_H_
#define BEEOS_SYNC_COND_H_

#include "spinlock.h"
#include "list.h"

struct cond {
    struct spinlock     lock;
    struct list_link    queue;
};

void cond_init(struct cond *cv);

void cond_wait(struct cond *cv);

void cond_signal(struct cond *cv);

void cond_broadcast(struct cond *cv);


#endif /* BEEOS_SYNC_COND_H_ */
