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

#include "spinlock.h"

void spinlock_init(struct spinlock *lock)
{
    lock->value = 0;
}

void spinlock_lock(struct spinlock *lock)
{
    while (__sync_lock_test_and_set(&lock->value, 1) != 0)
        ;
}

void spinlock_unlock(struct spinlock *lock)
{
    __sync_lock_release(&lock->value);
}
