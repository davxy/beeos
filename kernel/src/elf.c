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


#include "elf.h"
#include <stddef.h> /* NULL */
#include "arch/x86/vmem.h"

const char *elf_lookup_symbol(const struct elf_file *elf, uint32_t addr)
{
    int i;
    const char *name = "***";

    for (i = 0; i < elf->symtabsz/sizeof(struct elf_symbol_hdr); i++) {
        if ((addr >= elf->symtab[i].value) &&
            (addr < elf->symtab[i].value + elf->symtab[i].size)) {
            name = phys_to_virt((char *)elf->strtab + elf->symtab[i].name);
            break;
        }
    }
    return name;
}
