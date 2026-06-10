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

#ifndef BEEOS_NET_IP_H_
#define BEEOS_NET_IP_H_

#include <sys/types.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * Interface address, network byte order.
 * Fixed to the qemu user-mode network guest address (10.0.2.15) until
 * some interface configuration mechanism exists.
 */
#define IP_LOCAL_ADDR   htonl(0x0A00020FUL)

/**
 * Send an IPv4 packet: the payload is encapsulated in an ethernet
 * frame with a freshly built IP header and transmitted. The next-hop
 * MAC is obtained via ARP on the destination address itself, so only
 * on-link destinations are reachable (no routing).
 *
 * @param dst   Destination IPv4 address, network byte order.
 * @param proto Value for the IP header protocol field.
 * @param buf   Payload (e.g. a complete ICMP message).
 * @param size  Payload size.
 * @return      Payload bytes sent on success, negative errno on failure.
 */
int ip_send(in_addr_t dst, uint8_t proto, const void *buf, size_t size);

/**
 * Receive an IPv4 packet. Blocks until a frame addressed to
 * IP_LOCAL_ADDR with a matching protocol (and source address, if
 * `from` is not zero) arrives. Non-matching frames are discarded.
 *
 * @param buf   Destination buffer, receives the full IP packet
 *              (header included).
 * @param size  Buffer size; exceeding bytes are silently discarded.
 * @param proto IP protocol to match.
 * @param from  Source address to match, network byte order (0 = any).
 * @return      Number of bytes copied into buf, negative on error.
 */
ssize_t ip_recv(void *buf, size_t size, uint8_t proto, in_addr_t from);

/**
 * RFC 1071 internet checksum.
 *
 * @param data  Data to checksum (network byte order).
 * @param size  Data size in bytes.
 * @return      Checksum value, ready to be stored in a header.
 */
uint16_t ip_checksum(const void *data, size_t size);

#endif /* BEEOS_NET_IP_H_ */
