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

#ifndef BEEOS_NET_ETH_H_
#define BEEOS_NET_ETH_H_

#include <stdint.h>

#define ETH_ADDR_LEN    6
#define ETH_FRAME_MAX   1518

#define ETH_TYPE_IP     0x0800
#define ETH_TYPE_ARP    0x0806

struct eth_hdr {
    uint8_t  dmac[ETH_ADDR_LEN];
    uint8_t  smac[ETH_ADDR_LEN];
    uint16_t ethertype;             /* network byte order */
} __attribute__((packed));

#endif /* BEEOS_NET_ETH_H_ */
