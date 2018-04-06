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

#include "sys.h"
#include "proc.h"
#include "kmalloc.h"

int sys_sigreturn(void)
{
    if (!current->arch.sfr)
        return -1;

    memcpy(current->arch.ifr, current->arch.sfr,
            sizeof(struct isr_frame));
    kfree(current->arch.sfr, sizeof(struct isr_frame));
    current->arch.sfr = NULL;

    /* Return the result of the old stackframe */
    return current->arch.ifr->eax;
}
