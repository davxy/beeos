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

#ifndef BEEOS_NET_ARP_H_
#define BEEOS_NET_ARP_H_

#include <stdint.h>
#include <netinet/in.h>

/**
 * Resolve an IPv4 address into the corresponding MAC address.
 * Sends an ARP request and waits for the reply; the result is cached.
 *
 * @param ip    IPv4 address, network byte order.
 * @param mac   Output buffer for the resolved MAC (6 bytes).
 * @return      0 on success, negative errno on failure.
 */
int arp_resolve(in_addr_t ip, uint8_t *mac);

#endif /* BEEOS_NET_ARP_H_ */
