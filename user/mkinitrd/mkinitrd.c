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

#include "fs/initrd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *basenam(const char *path)
{
    char *name;

    return (name = strrchr(path, '/')) ? name+1 : path;
}

int main(int argc, char *argv[])
{
    char *buf;
    uint32_t nfiles = argc-1;
    char **names = &argv[1];
    size_t off = sizeof(struct initrd_header)
        + (nfiles-1)*sizeof(struct initrd_file_header);
    struct initrd_file_header *headers = 
        malloc(nfiles*sizeof(struct initrd_file_header));
    FILE *fpr, *fpw;
    int i, j = 0;

    for (i = 0; i < nfiles; i++) {
        fpr = fopen(names[i], "r");
        if (fpr == NULL) {
            printf("Error: file not found: %s\n", names[i]);
            continue;
        }
        fseek(fpr, 0, SEEK_END);
        headers[j].length = ftell(fpr);
        strncpy(headers[j].name, basenam(names[i]), sizeof(headers[j].name));
        headers[j].offset = off;
        fclose(fpr);
        off += headers[j].length;
        j++;
    }
    nfiles = j;

    fpw = fopen("initrd", "w");
    fwrite(&nfiles, sizeof(uint32_t), 1, fpw);
    fwrite(headers, sizeof(struct initrd_file_header), nfiles, fpw);

    for (i = 0; i < nfiles; i++) {
        buf = malloc(headers[i].length);
        printf("%-2d %-10s offset = %8d, size = %8d\n", i, headers[i].name,
                headers[i].offset, headers[i].length);
        fpr = fopen(names[i], "r");
        fread(buf, 1, headers[i].length, fpr);
        fwrite(buf, 1, headers[i].length, fpw);
        fclose(fpr);
        free(buf);
    }

    printf("initrd total size %d\n", ftell(fpw));

    fclose(fpw);
    free(headers);

    return 0;
}
