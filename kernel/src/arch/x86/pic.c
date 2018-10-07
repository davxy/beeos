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

#include "pic.h"
#include "io.h"
#include <stdint.h>

/* PIC ports */
#define PIC1_CMD        0x0020
#define PIC1_DATA       0x0021
#define PIC2_CMD        0x00A0
#define PIC2_DATA       0x00A1

/* PIC commands */
#define PIC_EOI         0x20

/*
 * Mask an IRQ
 */
void pic_mask(unsigned int n)
{
    uint8_t val;
    uint16_t port;

    if (n < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        n -= 8;
    }
    val = inb(port) | (1 << n);
    outb(port, val);
}

/*
 * Unmask an IRQ
 */
void pic_unmask(unsigned int n)
{
    uint8_t val;
    uint16_t port;

    if (n < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        n -= 8;
    }
    val = inb(port) & ~(1 << n);
    outb(port, val);
}

/*
 * PIC Initialization
 */
void pic_init(void)
{
    /* Remap the IRQ table from 32 to 47 */
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    /* Mask all IRQs masked */
    outb(PIC1_DATA, 0xFB);
    outb(PIC2_DATA, 0xFF);
}
