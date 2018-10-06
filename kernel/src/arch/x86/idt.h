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
 * Interrupt Descriptor Table
 * ==========================
 *
 * The Interrupt Descriptor Table (IDT) is data structure used by Intel x86
 * processors to implement an interrupt vector table.
 * The IDT is used by the processor to determine the correct response to
 * interrupts and exceptions.
 * In protected mode, the IDT is an array of 256 8-byte descriptors stored
 * consecutively in memory and indexed by an interrupt vector.
 * The descriptors contains memory addresses and a segment selector of code
 * to execute when the relative event happens. The segment selector must
 * point to a valid GDT entry.
 *
 *  63                              48 47            40 39             32
 * +----------------------------------+----------------+-----------------+
 * |            Base (16:31)          |      Flags     |      Zero       |
 * +----------------------------------+----------------+-----------------+
 *  31                              16 15                              0
 * +----------------------------------+---------------------------------+
 * |          Selector (0:15)         |           Base (0:15)           |
 * +----------------------------------+---------------------------------+
 *
 * Flags
 * -----
 *
 *    7
 *  +---+-----+---+-----------+
 *  | P | DPL | S |     T     |
 *  +---+-----+---+-----------+
 *
 * P: Present.
 *  Set to 0 for unused interrupts
 * DPL: Privilege.
 *  Specifies the minimum privilege level for the calling descriptor.
 * S: Storage Segment.
 *  Set to 0 for interrupt gates
 * T: Gate Type.
 *  0x0E for 32-bit interrupt gate
 *  0x0F for 32-bit trap gate
 *
 *
 * The first 32 IDT elements are reserved for processor exceptions.
 *
 * | NUM | Description                  |
 * |-----|------------------------------|
 * |   0 | Division by zero             |
 * |   1 | Debugger                     |
 * |   2 | Non maskable interrupt       |
 * |   3 | Breakpoint                   |
 * |   4 | Overflow                     |
 * |   5 | Bounds                       |
 * |   6 | Invalid opcode               |
 * |   7 | Coprocessor not available    |
 * |   8 | Double fault                 |
 * |   9 | Coprocessor segment overrun  |
 * |  10 | Invalid Task State Segment   |
 * |  11 | Segment not present          |
 * |  12 | Stack Fault                  |
 * |  13 | General protection fault     |
 * |  14 | Page fault                   |
 * |  15 | reserved                     |
 * |  16 | Math Fault                   |
 * |  17 | Alignment Check              |
 * |  18 | Machine Check                |
 * |  19 | SIMD Floating-Point Exception|
 * |  20 | Virtualization Exception     |
 * |  21 | Control Protection Exception |
 *
 * Entries 22 to 31 are reserved by Intel.
 */

#ifndef BEEOS_ARCH_X86_IDT_H_
#define BEEOS_ARCH_X86_IDT_H_

#include <stdint.h>

/** Interrupt descriptor table entry. */
struct idt_entry {
    uint16_t offset_lo;  /**< The lower 16 bits of the address to jump to. */
    uint16_t selector;   /**< Kernel segment selector. */
    uint8_t  zero;       /**< This must always be zero. */
    uint8_t  flags;      /**< More flags. See documentation. */
    uint16_t offset_hi;  /**< The upper 16 bits of the address to jump to. */
};


/**
 * A struct describing a pointer to an array of interrupt handlers.
 * This is in a format suitable for giving to 'lidt'.
 */
struct idt_register {
    uint16_t limit;
    uint16_t base_lo;    /* The lower 16 bits of the first entry address. */
    uint16_t base_hi;    /* The upper 16 bits of the first entry address. */
};


/**
 * Initialize the Interrupt Descriptor Table.
 */
void idt_init(void);


#endif /* BEEOS_ARCH_X86_IDT_H_ */
