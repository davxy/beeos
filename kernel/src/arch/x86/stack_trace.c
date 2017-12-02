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

#include "kprintf.h"
#include "elf.h"
#include <stdint.h>

extern struct elf_file kernel_elf;

void print_stack_trace()
{
    uint32_t *ebp, *eip;
    /* Get the current EBP value */
    asm volatile ("mov %%ebp, %0" : "=r"(ebp));
    while (ebp)
    {
        eip = ebp+1; 
        kprintf("    [0x%x] %s\n", *eip, "");//elf_lookup_symbol(&kernel_elf, *eip));
        ebp = (uint32_t *) *ebp;
    }
}
