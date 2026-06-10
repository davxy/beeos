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
 * Raw sockets only, two flavors:
 * - AF_PACKET maps straight to the network interface: each read
 *   returns one received ethernet frame, each write transmits one.
 * - AF_INET sends the payload wrapped in a kernel-built IP header to
 *   the connect()ed peer and receives whole IP packets matching the
 *   socket protocol (header included), as expected e.g. by ping.
 */

#include "ipc/socket.h"
#include "fs/vfs.h"
#include "proc.h"
#include "kmalloc.h"
#include "driver/e1000.h"
#include "net/ip.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>

struct socket_inode {
    struct inode base;
    int family;
    int type;
    int protocol;
    struct sockaddr addr;   /**< Peer address set by connect() */
    int connected;
};


static in_addr_t socket_peer(const struct socket_inode *snode)
{
    return ((const struct sockaddr_in *)&snode->addr)->sin_addr.s_addr;
}

static int socket_read(struct inode *inod, void *buf,
                       size_t count, size_t offset)
{
    const struct socket_inode *snode = (const struct socket_inode *)inod;

    if (snode->family == AF_PACKET)
        return e1000_read(buf, count);
    /* AF_INET */
    return ip_recv(buf, count, snode->protocol,
                   (snode->connected != 0) ? socket_peer(snode) : 0);
}

static int socket_write(struct inode *inod, const void *buf,
                        size_t count, size_t offset)
{
    const struct socket_inode *snode = (const struct socket_inode *)inod;

    if (snode->family == AF_PACKET)
        return e1000_write(buf, count);
    /* AF_INET */
    if (snode->connected == 0)
        return -EDESTADDRREQ;
    return ip_send(socket_peer(snode), snode->protocol, buf, count);
}


static const struct inode_ops socket_ops = {
    .read  = socket_read,
    .write = socket_write
};

static struct socket_inode *socket_inode_create(int family,
        int type, int protocol)
{
    struct socket_inode *snode;

    /* TODO... set a sb here to allow correct inode release (see pipe) */
    snode = (struct socket_inode *)kmalloc(sizeof(struct socket_inode), 0);
    if (snode == NULL)
        return NULL;
    memset(snode, 0, sizeof(*snode));
    snode->base.mode = S_IFSOCK | S_IRWXU | S_IRWXG | S_IRWXO;
    snode->base.ops = &socket_ops;
    snode->base.ref = 1;
    snode->family = family;
    snode->type = type;
    snode->protocol = protocol;
    return snode;
}


int socket_create(int family, int type, int protocol)
{
    int fd;
    struct inode *inod;
    struct file *file;
    struct dentry *dent;

    if (family != AF_PACKET && family != AF_INET)
        return -EAFNOSUPPORT;
    if (type != SOCK_RAW)
        return -ESOCKTNOSUPPORT;
    if (family == AF_INET && protocol <= 0)
        return -EPROTONOSUPPORT;

    for (fd = 0; fd < OPEN_MAX; fd++) {
        if (current->fds[fd].fil == NULL)
            break;
    }
    if (fd == OPEN_MAX)
        return -EMFILE; /* Too many open files */

    inod = (struct inode *)socket_inode_create(family, type, protocol);
    if (inod == NULL)
        return -ENOMEM;

    file = fs_file_alloc();
    if (file == NULL) {
        kfree(inod, sizeof(struct socket_inode));
        return -ENOMEM;
    }

    dent = dentry_create("", NULL, NULL);
    if (dent == NULL) {
        fs_file_free(file);
        kfree(inod, sizeof(struct socket_inode));
        return -ENOMEM;
    }
    dent->inod = idup(inod);

    file->flags = O_RDWR;
    file->ref = 1;
    file->off = 0;
    file->dent = dent;
    dent->ref = 1;
    current->fds[fd].fil = file;

    return fd;
}


int socket_connect(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen)
{
    struct socket_inode *snode;
    const struct file *fil;

    if (sockfd < 0 || sockfd >= OPEN_MAX ||
            current->fds[sockfd].fil == NULL)
        return -EBADF;
    fil = current->fds[sockfd].fil;
    if (!S_ISSOCK(fil->dent->inod->mode))
        return -ENOTSOCK;
    if (addr == NULL || addrlen <= 0 ||
            (size_t)addrlen > sizeof(struct sockaddr))
        return -EINVAL;

    snode = (struct socket_inode *)fil->dent->inod;
    if (snode->family == AF_PACKET)
        return -EOPNOTSUPP; /* Packet sockets are not connection oriented */
    if (addr->sa_family != snode->family)
        return -EAFNOSUPPORT;

    memcpy(&snode->addr, addr, addrlen);
    snode->connected = 1;
    return 0;
}
