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

#include <stdio.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    struct stat status;
    char buf[BUFSIZ];
    int n, i;
    int fd = 0;
    
    for (i = 1; i < argc; i++)
    {

        fd = open(argv[i], O_RDONLY, 0);
        if (fd < 0)
        {
            perror("cat");
            continue;
        }

        if (fstat(fd, &status) < 0)
        {
            perror("cat stat");
            continue;
        }

        if (S_ISDIR(status.st_mode))
        {
            errno = EISDIR;
            perror("cat");
            continue;
        }
        
        while ((n = read(fd, buf, BUFSIZ)) > 0)
            write(1, buf, n);
    
        close(fd);
    }

    return 0;
}
