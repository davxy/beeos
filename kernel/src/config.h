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

#ifndef _BEEOS_CONFIG_H_
#define _BEEOS_CONFIG_H_

#include <stdint.h>
#include <sys/types.h>

struct config_st
{
    unsigned int        cpu_count;   /**< Number of detected processors */
    volatile unsigned   cpu_active;  /**< Number of active processors */
    uintptr_t           base;        /**< Kernel base address */
    size_t              size;        /**< Kernel size */
//    uintptr_t           initrd_base; /**< Initrd base address */
//    size_t              initrd_size; /**< Initrd size */
};

struct config_st config;

#endif /* _BEEOS_CONFIG_H_ */
