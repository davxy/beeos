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

#include "net/arp.h"
#include "net/eth.h"
#include "net/ip.h"
#include "driver/e1000.h"
#include "kmalloc.h"
#include "timer.h"
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define ARP_HTYPE_ETH   1
#define ARP_OP_REQUEST  1
#define ARP_OP_REPLY    2

/* Reply wait timeout, in milliseconds */
#define ARP_TIMEOUT_MS  1000
/* Frames inspected while waiting for the reply before giving up */
#define ARP_MAX_FRAMES  64

struct arp_hdr {
    uint16_t htype;             /* hardware type */
    uint16_t ptype;             /* protocol type */
    uint8_t  hlen;              /* hardware address length */
    uint8_t  plen;              /* protocol address length */
    uint16_t oper;              /* operation */
    uint8_t  sha[ETH_ADDR_LEN]; /* sender hardware address */
    uint8_t  spa[4];            /* sender protocol address */
    uint8_t  tha[ETH_ADDR_LEN]; /* target hardware address */
    uint8_t  tpa[4];            /* target protocol address */
} __attribute__((packed));

struct arp_frame {
    struct eth_hdr eth;
    struct arp_hdr arp;
} __attribute__((packed));

/* Single entry cache, enough for the current single peer usage */
static in_addr_t cached_ip;
static uint8_t   cached_mac[ETH_ADDR_LEN];

int arp_resolve(in_addr_t ip, uint8_t *mac)
{
    struct arp_frame req;
    const struct arp_frame *rep;
    uint8_t *frame;
    in_addr_t spa;
    unsigned long deadline;
    ssize_t n;
    int i, res;

    if (cached_ip != 0 && cached_ip == ip) {
        memcpy(mac, cached_mac, ETH_ADDR_LEN);
        return 0;
    }

    memset(&req, 0, sizeof(req));
    res = e1000_mac(req.eth.smac);
    if (res < 0)
        return res;
    memset(req.eth.dmac, 0xFF, ETH_ADDR_LEN);   /* broadcast */
    req.eth.ethertype = htons(ETH_TYPE_ARP);
    req.arp.htype = htons(ARP_HTYPE_ETH);
    req.arp.ptype = htons(ETH_TYPE_IP);
    req.arp.hlen = ETH_ADDR_LEN;
    req.arp.plen = 4;
    req.arp.oper = htons(ARP_OP_REQUEST);
    memcpy(req.arp.sha, req.eth.smac, ETH_ADDR_LEN);
    spa = IP_LOCAL_ADDR;
    memcpy(req.arp.spa, &spa, 4);
    memcpy(req.arp.tpa, &ip, 4);

    n = e1000_write(&req, sizeof(req));
    if (n < 0)
        return n;

    frame = (uint8_t *)kmalloc(ETH_FRAME_MAX, 0);
    if (frame == NULL)
        return -ENOMEM;

    /* Wait for the reply, discarding everything else */
    res = -EHOSTUNREACH;
    deadline = (unsigned long)timer_ticks + msecs_to_ticks(ARP_TIMEOUT_MS);
    for (i = 0; i < ARP_MAX_FRAMES; i++) {
        if ((unsigned long)timer_ticks >= deadline)
            break;
        n = e1000_read_timeout(frame, ETH_FRAME_MAX,
                               deadline - (unsigned long)timer_ticks);
        if (n == -ETIMEDOUT)
            break;
        if (n < 0) {
            res = n;    /* e.g. -EINTR */
            break;
        }
        if ((size_t)n < sizeof(struct arp_frame))
            continue;
        rep = (const struct arp_frame *)frame;
        if (rep->eth.ethertype != htons(ETH_TYPE_ARP) ||
            rep->arp.oper != htons(ARP_OP_REPLY) ||
            memcmp(rep->arp.spa, &ip, 4) != 0)
            continue;
        memcpy(mac, rep->arp.sha, ETH_ADDR_LEN);
        cached_ip = ip;
        memcpy(cached_mac, mac, ETH_ADDR_LEN);
        res = 0;
        break;
    }
    kfree(frame, ETH_FRAME_MAX);
    return res;
}
