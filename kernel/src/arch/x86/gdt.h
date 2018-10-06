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

/**
 * Global Descriptor Table
 * =======================
 *
 * The Global Descriptor Table (GDT) is a data structure used by Intel x86
 * processors starting with 80286 to define characteristics of the various
 * memory areas used during program execution. These memory areas are called
 * segments in Intel terminology.
 * GDT can hold things other than segment descriptors as well. Every 8-byte
 * entry in the GDT is a descriptor, these can be Task State Segment (TSS)
 * descriptors, Local Descriptor table (LDT) descriptors, or Call Gate desc.
 *
 * On most modern Operating Systems segments are initialized to cover the
 * entire addressable memory, which make segment-relative addressing
 * transparent to the user.
 * Also note that to implement memory protection, segmentation has been
 * replaced by paging.
 *
 * To reference a segment, a program must use its index inside the GDT.
 * Such an index is called a *segment selector*. The selector must be loaded
 * into a *segment register* to be used.
 * Every machine instruction referencing memory has an implicit segment
 * register. The segment register can be explicitly overwritten by adding a
 * segment prefix before the instruction.
 *
 * Loading a selector into a segment register automatically reads the GDT
 * and stores properties within the processor itself. Subsequent modifications
 * of the GDT will not be effective unless the segment reg is reloaded.
 *
 *  63          56  55  52 51        48 47           40 39            32
 * +--------------+-------+------------+---------------+----------------+
 * | Base (24:31) | Flags | Lim (16:19)| Access Byte   |  Base (16:23)  |
 * +--------------+-------+------------+---------------+----------------+
 *  31                               16 15                             0
 * +-----------------------------------+--------------------------------+
 * |            Base (0:15)            |         Limit (0:15)           |
 * +-----------------------------------+--------------------------------+
 *
 * Note that modern operating systems, uses paging as memory protection
 * mechanism. GDT is set up to use flat seg
 *
 * Access byte
 * -----------
 *
 *   7                             0
 * +---+---+---+---+---+----+----+---+
 * | P |  DPL  | S | E | DC | RW | A |
 * +---+---+---+---+---+----+----+---+
 *
 * P: Present bit.
 *     This must be 1 for all valid selectors.
 * DPL: Privilege.
 *  Contains the ring level, 0 = highest, 3 = lowest.
 * E: Executable bit.
 *  If 1 code in this segment can be executed.
 * DC: Direction/Conforming bit.
 *     For data selectors:  tells the direction. 0 the segment grows up. 1 the
 *     segment grows down, i.e. the offset has to be greater than the limit
 *    For code selectors: if 1 code in this segment can be executed from an
 *    equal or lower privilege level. The DPL-bits represent the highest
 *    privilege level that is allowed to execute the segment.
 *    Note that the privilege level remains the same, i.e. a far-jump form ring
 *    3 to a dpl 2 segment remains in ring 3 after the jump.
 *    If 0 code in this segment can only be executed from the ring set in dpl.
 * RW: Readable/Writable bit.
 *  Readable bit for code selectors: Whether read access for this segment is
 *  allowed. Write access is never allowed for code segments.
 *  Writable bit for data selectors: Whether write access for this segment is
 *  allowed. Read access is always allowed for data segments.
 * A: Accessed bit.
 *     The CPU sets this to 1 when the segment is accessed.
 *
 * Flags
 * -----
 *
 *   7                              0
 * +----+----+----+----+----+----+----+----+
 * | Gr | Sz |         |     Limit High     |
 * +----+----+----+----+----+----+----+----+
 *
 * Gr: Granularity bit.
 *    If 0 the limit is in 1B blocks (byte granularity).
 *     If 1 the limit is in 4KB blocks (page granularity).
 * Sz: Size bit.
 *     If 0 the selector defines 16 bit protected mode.
 *     If 1 it defines 32 bit protected mode. You can have both 16 bit and 32 bit
 *     selectors at once.
 */

#ifndef BEEOS_ARCH_X86_GDT_H_
#define BEEOS_ARCH_X86_GDT_H_

#include <stdint.h>

/**
 * GDT entry structure
 */
struct gdt_entry {
    uint16_t    limit_lo;   /**< The lower 16 bits of the limit (15..0). */
    uint16_t    base_lo;    /**< The lowers 16 bits of the base (15..0). */
    uint8_t     base_mi;    /**< The next 8 bits of the base (23..16). */
    uint8_t     access;     /**< Pr, DPL, S, Ex, DC, RW, Ac. */
    uint8_t     flags;      /**< Gr, Sz, Limit High (19..16). */
    uint8_t     base_hi;    /**< The last 8 bits of the base (31..24). */
};

/**
 * This struct describes a GDT pointer. It points to the start of our array of
 * GDT entries, and is in the format required by the lgdt instruction.
 */
struct gdt_register {
    uint16_t    limit;      /**< Size of gdt table minus one. */
    uint16_t    base_lo;    /**< The lower 16 bits of the table address. */
    uint16_t    base_hi;    /**< The higher 16 bits of the table address. */
};

/**
 * Initialize the Global Descriptor Table.
 */
void gdt_init(void);


#endif /* BEEOS_ARCH_X86_GDT_H_ */
