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

#ifndef _BEEOS_CPUINFO_H_
#define _BEEOS_CPUINFO_H_

#include <stdint.h>

/** CPU information structure. */
struct cpuinfo_st
{
    char     vendor[16];
    uint32_t features[2];
    uint32_t signature;
    uint32_t brand;
};

extern struct cpuinfo_st cpuinfo;

/** Initialize the internal CPU info instance. */
void cpuinfo_init(void);

/** Crint CPU info structure members. */
void cpuinfo_print(void);


#endif /* _BEEOS_CPUINFO_H_ */
