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

#include "screen.h"
#include <string.h>

/*
 * Put a single character to the screen
 */
void screen_putchar(struct screen *scr, char c)
{
    unsigned int i;

    if (' ' <= c && c <= '~') {
        scr->buf[scr->pos_y * SCREEN_WIDTH + scr->pos_x] = c;
        scr->pos_x++;
    } else {
        switch (c) {
        case '\b':  /* backspace */
            if (scr->pos_x != 0)
                scr->pos_x--;
            break;
        case '\t':  /* tab */
            scr->pos_x = ((scr->pos_x + 4) & ~0x03);
            break;
        case '\n':
            scr->pos_y++;
            scr->pos_x = 0;
            break;
        case '\r':
            scr->pos_x = 0;
            break;
        default:
            break;
        }
    }

    if (scr->pos_x >= SCREEN_WIDTH) {
        /* Go to a new line */
        scr->pos_x = 0;
        scr->pos_y++;
    }

    if (scr->pos_y >= SCREEN_HEIGHT) {
        /* Scroll the screen */

        /* move every line up by one */
        for (i = 0; i < SCREEN_WIDTH * (SCREEN_HEIGHT - 1); i++)
            scr->buf[i] = scr->buf[i + SCREEN_WIDTH];
        /* clear the last line */
        for (; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
            scr->buf[i] = ' ';
        scr->pos_y = SCREEN_HEIGHT - 1;
    }

    /* Set the dirty flag */
    scr->dirty = 1;
}

/*
 * Write an array of characters to the screen.
 */
void screen_write(struct screen *scr, const char *buf, unsigned int n)
{
    unsigned int i;

    for (i = 0; i < n; i++)
        screen_putchar(scr, buf[i]);
}

/*
 * Console subsystem initialization.
 */
void screen_init(struct screen *scr)
{
    memset(scr->buf, ' ', sizeof(scr->buf));
    scr->pos_x = scr->pos_y = 0;
    scr->dirty = 1;
}
