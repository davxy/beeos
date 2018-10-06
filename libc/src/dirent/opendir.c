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

#include <dirent.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

DIR *opendir(const char *name)
{
    int fdn;
    DIR *dirp;
    struct stat st;

    /* Opendir is used only for reading */
    fdn = open(name, O_RDONLY, 0);
    if (fdn < 0)
        return NULL;    /* errno already set */

    if (fstat(fdn, &st) != 0) {
        close(fdn);     /* errno already set */
        return NULL;
    }

    /* Verify it is a directory */
    if (!S_ISDIR(st.st_mode)) {
        close(fdn);
        errno = ENOTDIR;
        return NULL;
    }

    dirp = malloc(sizeof(DIR));
    if (dirp == NULL) {
        close(fdn);
        errno = ENOMEM;
        return NULL;
    }

    dirp->fdn = fdn;
    memset(&dirp->dent, 0, sizeof(dirp->dent));

    return dirp;
}
