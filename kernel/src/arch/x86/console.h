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

#ifndef _BEEOS_ARCH_x86_CONSOLE_H_
#define _BEEOS_ARCH_x86_CONSOLE_H_

/**
 * Console subsystem initialization.
 *
 * Clears the screen and set the cursor position at the start position.
 */
void console_init(void);

/**
 * Write a character string to the console.
 *
 * @param buf   Characters string
 * @param n     String length
 */
void console_write(const char *buf, int n);

/**
 * Puts a single character to the console.
 *
 * @param c     Character to put
 */
void console_putchar(int c);


#endif /* _BEEOS_ARCH_X86_CONSOLE_H_ */
