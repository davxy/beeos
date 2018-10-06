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

#ifndef BEEOS_SYS_MOUNT_H_
#define BEEOS_SYS_MOUNT_H_

#include <unistd.h>


static inline int mount(const char *source, const char *target,
         const char *fs_type, unsigned long flags, const void *data)
{
    return syscall(__NR_mount, source, target, fs_type, flags, data);
}

#endif /* BEEOS_SYS_MOUNT_H_ */
