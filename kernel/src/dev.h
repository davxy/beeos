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
/** First pseudo-tty */
#define DEV_TTY                 0x0200
/** Current tty device */
#define DEV_CONSOLE             0x0500
#define DEV_CONSOLE1            0x0501
#define DEV_CONSOLE2            0x0502
#define DEV_CONSOLE3            0x0503
#define DEV_CONSOLE4            0x0504
/** @} */

/** Block devices @{ */
/** Initial ram-disk */
#define DEV_INITRD              0x01FA
/** @} */

#endif /* BEEOS_DEV_H_ */

