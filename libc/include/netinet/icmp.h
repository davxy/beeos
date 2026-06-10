#ifndef _NETINET_ICMP_H_
#define _NETINET_ICMP_H_

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>

struct icmphdr {
    uint8_t type; /* message type [1] */
    uint8_t code; /* type sub-code [2] */
    uint16_t checksum;
    union {
        struct {
            uint16_t id;
            uint16_t sequence;
        } __attribute__ ((packed)) echo;    /* echo datagram */
        uint32_t gateway;   /* gateway address */
        struct {
            uint16_t unused;
            uint16_t mtu;
        } __attribute__ ((packed)) frag;    /* path mtu discovery */
    } un;
} __attribute__ ((packed));

/*
 * [1] message types
 */
#define ICMP_ECHOREPLY          0  /* echo reply */
#define ICMP_DEST_UNREACH       3  /* destination unreachable */
#define ICMP_SOURCE_QUENCH      4  /* source quench */
#define ICMP_REDIRECT           5  /* redirect (change route) */
#define ICMP_ECHO               8  /* echo request */
#define ICMP_TIME_EXCEEDED      11 /* time exceeded */
#define ICMP_PARAMETERPROB      12 /* parameter problem */
#define ICMP_TIMESTAMP          13 /* timestamp request */
#define ICMP_TIMESTAMPREPLY     14 /* timestamp reply */
#define ICMP_INFO_REQUEST       15 /* information request */
#define ICMP_INFO_REPLY         16 /* information reply */
#define ICMP_ADDRESS            17 /* address mask request */
#define ICMP_ADDRESSREPLY       18 /* address mask reply */
#define NR_ICMP_TYPES           18 /* ICMP message types number */

/*
 * [2] type ICMP_DEST_UNREACH, code:
 */
#define ICMP_NET_UNREACH        0  /* network unreachable */
#define ICMP_HOST_UNREACH       1  /* host unreachable */
#define ICMP_PROT_UNREACH       2  /* protocol unreachable */
#define ICMP_PORT_UNREACH       3  /* port unreachable */
#define ICMP_FRAG_NEEDED        4  /* fragmentation needed/DF set */
#define ICMP_SR_FAILED          5  /* source route failed */
#define ICMP_NET_UNKNOWN        6  /* destination network unknown */
#define ICMP_HOST_UNKNOWN       7  /* destination host unknown */
#define ICMP_HOST_ISOLATED      8  /* source host isolated */
#define ICMP_NET_ANO            9  /* destination network prohibited */
#define ICMP_HOST_ANO           10 /* destination host prohibited */
#define ICMP_NET_UNR_TOS        11 /* network unreachable for this service */
#define ICMP_HOST_UNR_TOS       12 /* host unreachable  for this service */
#define ICMP_PKT_FILTERED       13 /* packet filtered */
#define ICMP_PREC_VIOLATION     14 /* precedence violation */
#define ICMP_PREC_CUTOFF        15 /* precedence cut off */
#define NR_ICMP_UNREACH         15 /* ICMP destination unreachable codes */

/*
 * [2] type ICMP_REDIRECT, code:
 */
#define ICMP_REDIR_NET          0  /* redirect net */
#define ICMP_REDIR_HOST         1  /* redirect host */
#define ICMP_REDIR_NETTOS       2  /* redirect net for TOS */
#define ICMP_REDIR_HOSTTOS      3  /* redirect host for TOS */

/*
 * [2] type ICMP_TIME_EXCEEDED, code:
 */
#define ICMP_EXC_TTL            0   /* TTL count exceeded */
#define ICMP_EXC_FRAGTIME       1   /* fragment reass time exceeded */

#endif /* _NETINET_ICMP_H_ */

