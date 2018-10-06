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

#ifndef BEEOS_DRIVER_SCREEN_H_
#define BEEOS_DRIVER_SCREEN_H_

#define SCREEN_WIDTH    80
#define SCREEN_HEIGHT   25

struct screen {
    unsigned int  pos_x;
    unsigned int  pos_y;
    unsigned char dirty;
    char buf[SCREEN_WIDTH * SCREEN_HEIGHT];
};

/**
 * Screen initialization.
 *
 * Clears the screen and set the cursor position at the start position.
 */
void screen_init(struct screen *scr);

/**
 * Write a character string to the screen.
 *
 * @param scr   Screen context.
 * @param buf   Characters string.
 * @param n     String length.
 */
void screen_write(struct screen *scr, const char *buf, unsigned int n);

/**
 * Puts a single character to the screen.
 *
 * @param scr   Screen context.
 * @param c     Character to put.
 */
void screen_putchar(struct screen *scr, char c);

/**
 * Update the screen with the buffer content and cursor.
 * (Arch specific)
 *
 * @param scr   Screen context.
 */
void screen_update(struct screen *scr);


#endif /* BEEOS_DRIVER_SCREEN_H_ */
