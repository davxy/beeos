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

/**
 * Linux device numbers list:
 * https://www.kernel.org/doc/Documentation/devices.txt
 */

#ifndef BEEOS_DEV_H_
#define BEEOS_DEV_H_

#include <sys/types.h>

/** Character devices @{ */
/** Physical memory */
#define DEV_MEM                 0x0101
/** Kernel virtual memory */
#define DEV_KMEM                0x0102
/** Null device */
#define DEV_NULL                0x0103
/** Null byte source */
#define DEV_ZERO                0x0105
/** Non deterministic random number generator */
#define DEV_RANDOM              0x0108
/** Faster, less secure random number generator */
#define DEV_URANDOM             0x0109
/** Current TTY console */
#define DEV_TTY0                0x0400
/** First TTY console */
#define DEV_TTY1                0x0401
/** Second TTY console */
#define DEV_TTY2                0x0402
/** Third TTY console */
#define DEV_TTY3                0x0403
/** Fourth TTY console */
#define DEV_TTY4                0x0404
/** Current TTY */
#define DEV_TTY                 0x0500
/** System console */
#define DEV_CONSOLE             0x0501
/** @} */

/** Block devices @{ */
/** Initial ram-disk */
#define DEV_INITRD              0x01FA
/** @} */

#endif /* BEEOS_DEV_H_ */
