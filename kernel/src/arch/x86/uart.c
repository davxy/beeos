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

#include "driver/uart.h"
#include "driver/tty.h"
#include "io.h"
#include "isr.h"


#define PORT 0x3f8


static int rx_ready(void)
{
    return inb(PORT + 5) & 0x01;
}

int uart_getchar(void)
{
    while (!rx_ready())
        ;
    return inb(PORT);
}

static int tx_ready(void)
{
    return inb(PORT +  5) & 0x20;
}

void uart_putchar(int c)
{
    while (!tx_ready())
        ;
    return outb(PORT, c);
}

static void uart_handler(void)
{
    char c;

    c = uart_getchar();
    tty_update(c);
}

void uart_init(void)
{
    outb(PORT + 1, 0x00);   /* Disable all interrupts */
    outb(PORT + 3, 0x80);   /* Enable DLAB (set baud rate divisor) */
    outb(PORT + 0, 0x03);   /* Set divisor to 3 (lo byte) 38400 baud */
    outb(PORT + 1, 0x00);   /*                  (hi byte) */
    outb(PORT + 3, 0x03);   /* Disable DLAB and set mode to 8N1 */
    outb(PORT + 2, 0xC7);   /* Enable FIFO, clear them, with 14-byte thresh */
    outb(PORT + 4, 0x0B);   /* IRQ enabled, RTS/DSR set */

    isr_register_handler(ISR_COM1, uart_handler);

    outb(PORT + 1, 0x01);   /* Enable all interrupts */
}
