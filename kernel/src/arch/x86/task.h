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

#ifndef BEEOS_ARCH_X86_TASK_H_
#define BEEOS_ARCH_X86_TASK_H_

#include "paging.h"
#include "isr.h"

#define KSTACK_SIZE     0x1000

struct task_arch
{
    volatile uint32_t   pgdir;  /**< Page directory physical address */
    uint32_t            eip;
    uint32_t            esp;
    uint32_t            ebp;
    struct isr_frame    *ifr;
    struct isr_frame    *sfr;
};

#endif /* BEEOS_ARCH_X86_TASK_H_ */

