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

#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

char buf[BUFSIZ];

int main(int argc, char **argv)
{
    int fd;

    /*
     * With O_APPEND, before every WRITE the file offset is positioned
     * to the end. The [seek,write] are executed as an atomic operation.
     */
    fd = open("file.tmp", O_RDWR | O_APPEND, 0);
    if (fd < 0)
        err_sys("open error");

    if (write(fd, "Hello", 5) != 5)
        err_sys("write error");

    lseek(fd, 0, SEEK_SET);     /* position the offset to the beginning */
    if (read(fd, buf, 4) != 4)  /* reads four bytes from the beginning */
        err_sys("read error");

    /* reposition offset to the end before write */
    if (write(fd, "World", 5) != 5)
        err_sys("write error");

    if (read(fd, buf, 4) != 0)   /* returns 0 */
        err_sys("read error");

    /*
     * We cannot replace data if the file was opened with O_APPEND,
     * The offset is always placed to the end before write
     */
    lseek(fd, 0, SEEK_SET);
    write(fd, "XXXX", 4);

    close(fd);

    return 0;
}
