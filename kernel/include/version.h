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

#ifndef _BEEOS_VERSION_H_
#define _BEEOS_VERSION_H_

#define BEEOS_MAJOR       0
#define BEEOS_MINOR       0
#define BEEOS_PATCH       2
#define BEEOS_VERSION \
    (((BEEOS_MAJOR) << 16) + ((BEEOS_MINOR) << 8) + (BEEOS_PATCH))

#define BEEOS_CODENAME    "stoneage"

#endif /* _BEEOS_VERSION_H_ */
