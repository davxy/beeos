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

#include "io.h"
#include "vmem.h"
#include "driver/screen.h"
#include <stdint.h>

#define VIDEO_BUF   ((uint16_t *) (0xB8000 + KVBASE))

#define BLACK       0
#define LIGHT_GREY  7

#define MAKE_COLOR(bg, fg)      (((bg) << 4) | (fg))
#define MAKE_ENTRY(bg, fg, c)   ((MAKE_COLOR((bg), (fg)) << 8) | (c))

/*
 * Copy the backbuffer and update the hardware cursor.
 *
 * The framebuffer has two I/O ports, one for accepting the data, and one
 * for describing the data being received. Port 0x03D4 is the port that
 * describes the data and port 0x03D5 is for the data itself.
 */
void screen_update(struct screen *scr)
{
    int i;
    uint16_t pos = scr->pos_y * SCREEN_WIDTH + scr->pos_x;
    uint16_t *buf = (uint16_t *)VIDEO_BUF;

    /* Copy the backbuffer */
    for (i = 0; i < sizeof(scr->buf); i++)
        buf[i] = MAKE_ENTRY(BLACK, LIGHT_GREY, scr->buf[i]);

    /* Update the hardware cursor */
    outb(0x03D4, 14);  /* the highest 8 bits of the position */
    outb(0x03D5, pos >> 8);
    outb(0x03D4, 15);  /* the lowest 8 bits of the position */
    outb(0x03D5, pos);

    scr->dirty = 0;
}
