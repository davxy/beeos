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

#include "dev.h"
#include "driver/tty.h"
#include "proc.h"
#include <errno.h>

ssize_t dev_io_tty(pid_t pid, dev_t dev, int rw, off_t off, 
        void *buf, size_t size, int *eof)
{
    ssize_t n;
    if (rw == DEV_READ)
    {
        int key;
        uint8_t *buf8 = (uint8_t *)buf;
        n = 0;
        while (n < size)
        {
            key = tty_read(dev, (n == 0));
            if (key == 0 && n == 0)
            {
                /* A single line contains zero: this is made by a VEOF
                 * character (^D), that is, the input is closed. */
                *eof = 1;
                break;
            }
            else if (key == -1 && n == 0)
                /* At the moment, there is just nothing to read. */
                return -EAGAIN;
            else if (key == -1 && n > 0)
                /* Finished to read */
                break;
            else
                buf8[n] = key;
            n++;
        }
    }
    else
        n = tty_write(buf, size);

    return n;
}
