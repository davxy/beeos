#ifndef _SYS_SOCKET_H_
#define _SYS_SOCKET_H_

#include <sys/types.h>
#include <unistd.h>

typedef int socklen_t;
typedef uint16_t sa_family_t; /* Address family */

#define SOCK_STREAM     1   /**< Byte stream socket */
#define SOCK_DGRAM      2   /**< Datagram socket */
#define SOCK_RAW        3   /**< Raw protocol interface */
#define SOCK_SEQPACKET  4   /**< Sequenced-packet socket */

/*
 * Protocol families
 */
#define PF_UNPEC        0   /**< Unspecified */
#define PF_UNIX         1   /**< Unix domain socket */
#define PF_INET         2   /**< IPv4 protocol family */
#define PF_PACKET       5   /**< Low level packet interface (no POSIX) */

/*
 * Address families
 */
#define AF_UNSPEC   PF_UNSPEC   /**< Unspecified */
#define AF_UNIX     PF_UNIX     /**< Unix domain socket */
#define AF_INET     PF_INET     /**< IPv4 protocol family */
#define AF_PACKET   PF_PACKET   /**< Low level packet interface (no POSIX) */

/** Generic socket address */
struct sockaddr {
    unsigned short int sa_family;   /**< Address family */
    unsigned char sa_data[14];      /**< Socket address */
};


static inline int socket(int domain, int type, int protocol)
{
    return syscall(__NR_socket, domain, type, protocol);
}

static inline int connect(int sockfd, const struct sockaddr *addr,
            socklen_t addrlen)
{
    return syscall(__NR_connect, sockfd, addr, addrlen);
}

/* Temporary... */
static inline ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    return write(sockfd, buf, len);
}

/* Temporary... */
static inline ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    return read(sockfd, buf, len);
}

#endif /* _SYS_SOCKET_H_ */
