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
#include <string.h>

int main(int argc, char **argv)
{
    int fd, n;
    char buf[256] = {0};
    char *data = "Hello, World";

    fd = open("data.tmp", O_RDWR|O_CREAT|O_TRUNC, 0);
    if (fd < 0)
        err_sys("open");

    n = strlen(data);
    if (write(fd, data, n) != n)
        err_sys("write");
    if (read(fd, buf, n) != n)
        err_sys("read");

    return 0;
}
