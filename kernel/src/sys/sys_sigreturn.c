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

#include "proc.h"
#include "kmalloc.h"

int sys_sigreturn(void)
{
    if (!current_task->arch.sfr)
        return -1;

    memcpy(current_task->arch.ifr, current_task->arch.sfr,
            sizeof(struct isr_frame));
    kfree(current_task->arch.sfr, sizeof(struct isr_frame));
    current_task->arch.sfr = NULL;

    /* Return the result of the old stackframe */
#ifndef __arm__
    return current_task->arch.ifr->eax;
#else
    return -1;
#endif
}
