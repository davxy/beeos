/*
 * Copyright (c) 2015-2017, Davide Galassi. All rights reserved.
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
#include <stdint.h>

#define VIDEO_BUF	((uint16_t *) (0xB8000 + KVBASE))

#define BLACK			0
#define BLUE 			1
#define GREEN			2
#define CYAN			3
#define RED 			4
#define MAGENTA			5
#define BROWN 			6
#define LIGHT_GREY 		7
#define DARK_GREY 		8
#define LIGHT_BLUE 		9
#define LIGHT_GREEN 	10
#define LIGHT_CYAN 		11
#define LIGHT_RED 		12
#define LIGHT_MAGENTA 	13
#define LIGHT_BROWN		14
#define WHITE			15

#define MAKE_COLOR(bg, fg) 		((bg << 4) | fg)
#define MAKE_ENTRY(bg, fg, c)	((MAKE_COLOR(bg, fg) << 8) | c)

#define VGA_WIDTH		80
#define VGA_HEIGHT		25

static unsigned pos_x;
static unsigned pos_y;

/*
 * Updates the hardware cursor. 
 *
 * The framebuffer has two I/O ports, one for accepting the data, and one 
 * for describing the data being received. Port 0x03D4 is the port that 
 * describes the data and port 0x03D5 is for the data itself.
 */
static void cursor_update(void)
{
	uint16_t pos = pos_y * VGA_WIDTH + pos_x;

	outb(0x03D4, 14);  /* the highest 8 bits of the position */
	outb(0x03D5, pos >> 8);
	outb(0x03D4, 15);  /* the lowest 8 bits of the position */
	outb(0x03D5, pos);
}

/*
 * Put a single character to the console
 */
void console_putchar(int c)
{
	uint16_t *buf = VIDEO_BUF;

	if (' ' <= c && c <= '~')
	{
		buf[pos_y*80 + pos_x] = MAKE_ENTRY(BLACK, WHITE, c);
		pos_x++;
	}
	else
	{
		switch (c)
		{
			case '\b':	/* backspace */
				if (pos_x != 0)
					pos_x--;
			break;

			case '\t':	/* tab */
				pos_x = (pos_x+4) & ~(4-1);
			break;

			case '\n':
				pos_y++;
				pos_x = 0;
			break;

			case '\r':
				pos_x = 0;
			break;

			default:
			break;
		}
	}

	if (pos_x == VGA_WIDTH)
	{
		/* Go to a new line */
		pos_x = 0;
		pos_y++;
	}

	if (pos_y == VGA_HEIGHT)
	{
		/* Scroll the screen */
		int i;
		/* move every line up by one */
		for (i = 0; i < VGA_WIDTH*(VGA_HEIGHT-1); i++)
			buf[i] = buf[i+80];
		/* clear the last line */
		for (; i < VGA_WIDTH*VGA_HEIGHT; i++)
			buf[i] = MAKE_ENTRY(BLACK, WHITE, ' ');
		pos_y--;
	}

	cursor_update();	/* move the hardware cursor */
}

/*
 * Write an array of characters to the screen.
 */
void console_write(const char *buf, int n)
{
	int i;

	for (i = 0; i < n; i++)
		console_putchar(buf[i]);
}

/*
 * Console subsystem initialization.
 */
void console_init(void)
{
	int i;
	unsigned short *buf = VIDEO_BUF;

	for (i = 0; i < VGA_WIDTH*VGA_HEIGHT; i++)
		buf[i] = MAKE_ENTRY(BLACK, WHITE, ' ');

	pos_x = pos_y = 0;
	cursor_update();
}
