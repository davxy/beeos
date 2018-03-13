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

/**
 * Linux device numbers list:
 * https://www.kernel.org/doc/Documentation/devices.txt
 */

#ifndef BEEOS_DEV_H_
#define BEEOS_DEV_H_

#include <sys/types.h>

/** Character devices @{ */
/** Physical memory */
#define DEV_MEM                 makedev(0x01, 0x01)
/** Kernel virtual memory */
#define DEV_KMEM                makedev(0x01, 0x02)
/** Null device */
#define DEV_NULL                makedev(0x01, 0x03)
/** Null byte source */
#define DEV_ZERO                makedev(0x01, 0x05)
/** Non deterministic random number generator */
#define DEV_RANDOM              makedev(0x01, 0x08)
/** Faster, less secure random number generator */
#define DEV_URANDOM             makedev(0x01, 0x09)
/** Current TTY console */
#define DEV_TTY0                makedev(0x04, 0x00)
/** First TTY console */
#define DEV_TTY1                makedev(0x04, 0x01)
/** Second TTY console */
#define DEV_TTY2                makedev(0x04, 0x02)
/** Third TTY console */
#define DEV_TTY3                makedev(0x04, 0x03)
/** Fourth TTY console */
#define DEV_TTY4                makedev(0x04, 0x04)
/** Current TTY */
#define DEV_TTY                 makedev(0x05, 0x00)
/** System console */
#define DEV_CONSOLE             makedev(0x05, 0x01)
/** @} */

/** Block devices @{ */
/** Initial ram-disk */
#define DEV_INITRD              0x01FA
/** @} */

#endif /* BEEOS_DEV_H_ */

