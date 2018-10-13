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
#include "FILE.h"

FILE stdio_streams[3] = {
    { 0, 0, _IOLBF, BUFSIZ, 0, 0, (char *)0, (char *)0 },
    { 1, 0, _IOLBF, BUFSIZ, 0, 0, (char *)0, (char *)0 },
    { 2, 0, _IONBF, 1, 0, 0, (char *)0, (char *)0 }
};

FILE *stdin  = &stdio_streams[0];
FILE *stdout = &stdio_streams[1];
FILE *stderr = &stdio_streams[2];
