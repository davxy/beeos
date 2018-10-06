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

#ifndef BEEOS_ARCH_X86_VMEM_H_
#define BEEOS_ARCH_X86_VMEM_H_

/*
 * Known virtual addresses
 */
#define KVBASE      0xC0000000  /**< Upper half virtual address */
#define KVADDR      0xC0100000  /**< Kernel start virtual address */
#define UVADDR      0x08000000  /**< User code stub virtual address */


#ifndef __ASSEMBLER__

/*
 * phys_to_virt and virt_to_phys functions can be safely used only for
 * addresses within the LOW_MEM zone. This is due to the fact that only
 * LOW_MEM addresses are identity mapped.
 */

/**
 * Convert a physical address to a virtual one
 * @param addr  Physical address.
 * @return      Virtual address.
 */
static inline void *phys_to_virt(void *addr)
{
    return (char *)addr + KVBASE;
}

/**
 * Convert a physical address to a virtual one
 * @param addr  Physical address.
 * @return      Virtual address.
 */
static inline void *virt_to_phys(void *addr)
{
    return (char *)addr - KVBASE;
}

#endif /* __ASSEMBLER__ */

#endif /* BEEOS_ARCH_X86_VMEM_H_ */
