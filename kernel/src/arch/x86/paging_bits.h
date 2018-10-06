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

/*
 * Bits, values and flags used by paging code.
 */

#ifndef BEEOS_ARCH_X86_PAGING_BITS_H_
#define BEEOS_ARCH_X86_PAGING_BITS_H_

/*
 * Memory page size
 */
#define PAGE_SIZE       0x1000

/*
 * Control Register flags
 */
#define CR0_PE          0x00000001      /* Protection Enable */
#define CR0_MP          0x00000002      /* Monitor coProcessor */
#define CR0_EM          0x00000004      /* Emulation */
#define CR0_TS          0x00000008      /* Task Switched */
#define CR0_ET          0x00000010      /* Extension Type */
#define CR0_NE          0x00000020      /* Numeric Errror */
#define CR0_WP          0x00010000      /* Write Protect */
#define CR0_AM          0x00040000      /* Alignment Mask */
#define CR0_NW          0x20000000      /* Not Writethrough */
#define CR0_CD          0x40000000      /* Cache Disable */
#define CR0_PG          0x80000000      /* Paging */
#define CR4_PSE         0x00000010      /* Page size extension */

/*
 * Page table/directory entry flags
 */
#define PTE_P           0x00000001      /* Present */
#define PTE_W           0x00000002      /* Writeable */
#define PTE_U           0x00000004      /* User */
#define PTE_PS          0x00000080      /* Page size, if set 4MB else 4KB */
#define PTE_MASK        0xFFFFF000      /* Page pysical address mask */

#endif /* BEEOS_ARCH_X86_PAGING_BITS_H_ */
