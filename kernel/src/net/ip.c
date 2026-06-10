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

#include "net/ip.h"
#include "net/eth.h"
#include "net/arp.h"
#include "driver/e1000.h"
#include "kmalloc.h"
#include "util.h"
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>

uint16_t ip_checksum(const void *data, size_t size)
{
    uint32_t sum = 0;
    const uint16_t *ptr = (const uint16_t *)data;

    while (size > 1) {
        sum += *ptr++;
        size -= 2;
    }
    if (size > 0)
        sum += *(const uint8_t *)ptr;
    while ((sum >> 16) != 0)
        sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)~sum;
}

int ip_send(in_addr_t dst, uint8_t proto, const void *buf, size_t size)
{
    uint8_t dmac[ETH_ADDR_LEN];
    uint8_t *frame;
    struct eth_hdr *eth;
    struct iphdr *ip;
    static uint16_t id;
    size_t len;
    ssize_t n;
    int res;

    if (size > ETH_FRAME_MAX - sizeof(struct eth_hdr) - sizeof(struct iphdr))
        return -EMSGSIZE;

    res = arp_resolve(dst, dmac);
    if (res < 0)
        return res;

    len = sizeof(struct eth_hdr) + sizeof(struct iphdr) + size;
    frame = (uint8_t *)kmalloc(len, 0);
    if (frame == NULL)
        return -ENOMEM;

    eth = (struct eth_hdr *)frame;
    memcpy(eth->dmac, dmac, ETH_ADDR_LEN);
    (void)e1000_mac(eth->smac);     /* cannot fail, arp went through */
    eth->ethertype = htons(ETH_TYPE_IP);

    ip = (struct iphdr *)(frame + sizeof(struct eth_hdr));
    ip->version = IPVERSION;
    ip->ihl = sizeof(struct iphdr) / 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(struct iphdr) + size);
    ip->id = htons(id++);
    ip->frag_off = 0;
    ip->ttl = IPDEFTTL;
    ip->protocol = proto;
    ip->check = 0;
    ip->saddr = IP_LOCAL_ADDR;
    ip->daddr = dst;
    ip->check = ip_checksum(ip, sizeof(struct iphdr));

    memcpy(frame + sizeof(struct eth_hdr) + sizeof(struct iphdr),
           buf, size);

    n = e1000_write(frame, len);
    kfree(frame, len);
    if (n < 0)
        return n;
    return size;
}

ssize_t ip_recv(void *buf, size_t size, uint8_t proto, in_addr_t from)
{
    uint8_t *frame;
    const struct eth_hdr *eth;
    const struct iphdr *ip;
    ssize_t n;
    size_t len;

    frame = (uint8_t *)kmalloc(ETH_FRAME_MAX, 0);
    if (frame == NULL)
        return -ENOMEM;

    for (;;) {
        n = e1000_read(frame, ETH_FRAME_MAX);
        if (n < 0)
            break;
        if ((size_t)n < sizeof(struct eth_hdr) + sizeof(struct iphdr))
            continue;
        eth = (const struct eth_hdr *)frame;
        if (eth->ethertype != htons(ETH_TYPE_IP))
            continue;
        ip = (const struct iphdr *)(frame + sizeof(struct eth_hdr));
        if (ip->protocol != proto || ip->daddr != IP_LOCAL_ADDR)
            continue;
        if (from != 0 && ip->saddr != from)
            continue;

        len = ntohs(ip->tot_len);
        if (len > (size_t)n - sizeof(struct eth_hdr))
            len = n - sizeof(struct eth_hdr);
        len = MIN(len, size);
        memcpy(buf, frame + sizeof(struct eth_hdr), len);
        n = len;
        break;
    }
    kfree(frame, ETH_FRAME_MAX);
    return n;
}
