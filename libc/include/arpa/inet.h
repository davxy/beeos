#ifndef _ARPA_INET_H_
#define _ARPA_INET_H_

#include <stdint.h>
#include <sys/socket.h>

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

const char *inet_ntop (int family, const void *src,
                       char *dst, socklen_t size);

int inet_pton (int family, const char *src, void *dst);

#endif /* _ARPA_INET_H_ */
