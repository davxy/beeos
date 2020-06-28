#ifndef _NETINET_IN_H_
#define _NETINET_IN_H_

#include <stdint.h>
#include <sys/socket.h>

typedef uint16_t in_port_t;     /* Port number. [1] */
typedef uint32_t in_addr_t;     /* IPv4 address. */

/*
 * [1] Types `in_port_t' and `in_addr_t' are to be intended for network
 * byte order IPv4 integer address, at least because this type is used
 * inside the type `struct in_addr', that is surely in network byte order.
 * But attention must be made to mistakes: for example, inside the file
 * <netinet/in.h> from GNU sources, there are some macro defining default
 * netmask like this:
 *
 * #define IN_CLASSA(a) ((((in_addr_t)(a)) & 0x80000000) == 0)
 * #define IN_CLASSB(a) ((((in_addr_t)(a)) & 0xc0000000) == 0x80000000)
 * #define IN_CLASSC(a) ((((in_addr_t)(a)) & 0xe0000000) == 0xc0000000)
 *
 * Such macro can work only if the architecture is big-endian.
 */
 
/* IPv4 address. */
struct in_addr {
    in_addr_t s_addr;
};

/* struct sockaddr_in, members in *network*byte*order*. */
struct sockaddr_in {
    sa_family_t sin_family;   /* AF_INET. */
    in_port_t sin_port;       /* Port number. */
    struct in_addr sin_addr;  /* IP address. */
    uint8_t sin_zero[8];      /* [2] */
};

/*
 * [2] The type `struct sockaddr_in' must be replaceable with the type
 * `struct sockaddr', with a cast. So it is necessary to fill the unused
 * space with a filler.
 */

/* IPv6 address, network byte order */
struct in6_addr {
    uint8_t s6_addr[16];
};

/* struct sockaddr_in6, members in network byte order */
struct sockaddr_in6 {
    sa_family_t sin6_family;    /* AF_INET6. */
    in_port_t sin6_port;        /* Port number. */
    uint32_t sin6_flowinfo;     /* IPv6 traffic class and flow info. */
    struct in6_addr sin6_addr;  /* IPv6 address. */
    uint32_t sin6_scope_id;     /* Set of interfaces for a scope. */
};

/*
 * external in6_addr in6addr_any;
 * #define IN6ADDR_ANY_INIT  ...
 * external struct in6_addr in6addr_loopback;
 * #define IN6ADDR_LOOPBACK_INIT ...
 */
struct ipv6_mreq {
  struct in6_addr ipv6mr_multiaddr;     /* IPv6 multicast address */
  unsigned int ipv6mr_interface;        /* Interface index */
};

#define IPPROTO_IP      0       /* Internet protocol */
#define IPPROTO_ICMP    1       /* Contro message protocol. */
#define IPPROTO_TCP     6       /* Transmission control protocol */
#define IPPROTO_UDP    17       /* User datagram protocol */
#define IPPROTO_IPV6   41       /* Internet protocol version 6 */
#define IPPROTO_RAW   255       /* Raw IP packets protocol */

/* 0.0.0.0 */
#define INADDR_ANY         ((in_addr_t) 0x00000000)
/* 255.255.255.255 */
#define INADDR_BROADCAST   ((in_addr_t) 0xffffffff)
/* 127.0.0.1 */
#define INADDR_LOOPBACK    ((in_addr_t) 0x7f000001)

#define INET_ADDRSTRLEN  16     /* IPv4 address string size */
#define INET6_ADDRSTRLEN 46     /* IPv6 address string size */
#endif /* _NETINET_IN_H_ */

