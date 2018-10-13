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

#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

static void filecopy(FILE *out, FILE *in)
{
    size_t nr, nw;
    char buf[32];
 
    while ((nr = fread(buf, 1, sizeof(buf), in)) > 0) {
        if ((nw = fwrite(buf, 1, nr, out)) != nr)
            break;
    }
    if (ferror(out))
        perror("cat");
}

int main(int argc, char *argv[])
{
    struct stat status;
    int i;
    FILE *fp;

    if (argc == 1) {
        filecopy(stdout, stdin);
    } else {
        for (i = 1; i < argc; i++) {
            if (stat(argv[i], &status) < 0) {
                perror("cat stat");
                continue;
            }
            if (S_ISDIR(status.st_mode)) {
                errno = EISDIR;
                perror("cat");
                continue;
            }
            if ((fp = fopen(argv[i], "r")) == NULL) {
                perror("cat");
                continue;
            }
            filecopy(stdout, fp);
            fclose(fp);
        }
    }
    return 0;
}
