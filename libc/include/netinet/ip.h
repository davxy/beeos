#ifndef _NETINET_IP_H_
#define _NETINET_IP_H_

#include <netinet/in.h>

struct iphdr {
  uint16_t  ihl : 4,        /* header length / 4 */
            version : 4;    /* IP version */
  uint8_t   tos;            /* type of service */
  uint16_t  tot_len;        /* total packet length */
  uint16_t  id;             /* identification */
  uint16_t  frag_off;       /* fragment offset field */
  uint8_t   ttl;            /* time to live */
  uint8_t   protocol;       /* contained protocol */
  uint16_t  check;          /* header checksum */
  in_addr_t saddr;          /* source IP address */
  in_addr_t daddr;          /* destination IP address */
  /* Options after this point. */
};

#define IPVERSION       4       // IP version number
#define IP_MAXPACKET    65535   // maximum packet size
#define MAXTTL          255     // maximum time to live (seconds)
#define IPDEFTTL        64      // default ttl, from RFC 1340
#define IPFRAGTTL       60      // time to live for fragments
#define IPTTLDEC        1       // subtracted when forwarding
#define IP_MSS          576     // default maximum segment size

#endif /* _NETINET_IP_H_ */
