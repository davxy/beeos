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
 * This family of functions is used to do low-level port input and output.
 * The out* functions do port output, the in* functions do port input.
 * The b-suffix functions are byte-width, the w-suffix functions are
 * word-width, the l-suffix functions are long-width.
 */

#ifndef BEEOS_ARCH_X86_IO_H_
#define BEEOS_ARCH_X86_IO_H_

#include <stdint.h>


#define outb_asm(port, val) \
    asm volatile("out %w1, %b0" : : "a"(val), "Nd"(port))

#define inb_asm(port, val) \
    asm volatile("in %b0, %w1" : "=a"(val) : "Nd"(port))


#define outw_asm(port, val) \
    asm volatile("out %w1, %w0" : : "a"(val), "Nd"(port))

#define inw_asm(port, val) \
    asm volatile("in %w0, %w1" : "=a"(val) : "Nd"(port))


#define outl_asm(port, val) \
    asm volatile("out %w1, %d0" : : "a"(val), "Nd"(port))

#define inl_asm(port,val) \
    asm volatile("in %d0, %w1" : "=a"(val) : "Nd"(port))



/*
 * Write one byte to an output port
 *
 * @param port  Port address
 * @param val   Value
 */
static inline void outb(uint16_t port, uint8_t val)
{
    outb_asm(port, val);
}

/*
 * Read one byte from an input port
 *
 * @param port  Port address
 * @return      Read value
 */
static inline uint8_t inb(uint16_t port)
{
    uint8_t val;

    inb_asm(port, val);
    return val;
}

/*
 * Write a 16-bits word to an output port
 *
 * @param port  Port address
 * @param val   Value
 */
static inline void outw(uint16_t port, uint16_t val)
{
    outw_asm(port, val);
}

/*
 * Read a 16-bits word from an input port
 *
 * @param port  Port address
 * @return      Read value
 */
static inline uint16_t inw(uint16_t port)
{
    uint16_t val;

    inw_asm(port, val);
    return val;
}

/*
 * Write a 32-bits long word to an output port
 *
 * @param port  Port address
 * @param val   Value
 */
static inline void outl(uint16_t port, uint32_t val)
{
    outl_asm(port, val);
}

/*
 * Read a 32-bits long word from an input port
 *
 * @param port  Port address
 * @return      Read value
 */
static inline uint32_t inl(uint16_t port)
{
    uint32_t val;

    inl_asm(port, val);
    return val;
}

#endif /* BEEOS_ARCH_X86_IO_H_ */
