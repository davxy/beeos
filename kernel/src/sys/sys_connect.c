#include "sys.h"
#include "ipc/socket.h"

int sys_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return socket_connect(sockfd, addr, addrlen);
}
