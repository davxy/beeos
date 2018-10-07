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

#ifndef BEEOS_DRIVER_RAMDISK_H_
#define BEEOS_DRIVER_RAMDISK_H_

#include <sys/types.h>


void ramdisk_init(void *addr, size_t size);

ssize_t ramdisk_read(void *buf, size_t size, size_t off);

ssize_t ramdisk_write(const void *buf, size_t size, size_t off);


#endif /* BEEOS_DRIVER_RAMDISK_H_ */
