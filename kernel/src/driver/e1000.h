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

/*
 * Intel PRO/1000 (e1000) ethernet driver.
 *
 * The device is exposed as a raw packet interface: each read returns
 * one received ethernet frame, each write transmits one frame.
 */

#ifndef BEEOS_DRIVER_E1000_H_
#define BEEOS_DRIVER_E1000_H_

#include <sys/types.h>
#include <stddef.h>

/**
 * Look for an e1000 device on the PCI bus and initialize it.
 *
 * @return  0 on success, -1 if the device is not present or the
 *          initialization failed.
 */
int e1000_init(void);

/**
 * Read a single received frame.
 * Blocks until a frame is available. If the frame is bigger than the
 * supplied buffer the exceeding part is silently discarded.
 *
 * @param buf   Destination buffer.
 * @param size  Buffer size.
 * @return      Number of bytes copied into buf, negative on error.
 */
ssize_t e1000_read(void *buf, size_t size);

/**
 * Transmit a single frame.
 *
 * @param buf   Frame data (starting with the ethernet header).
 * @param size  Frame size.
 * @return      Number of transmitted bytes, negative on error.
 */
ssize_t e1000_write(const void *buf, size_t size);

#endif /* BEEOS_DRIVER_E1000_H_ */
