#include "sys.h"
#include "ipc/socket.h"

int sys_socket(int domain, int type, int protocol)
{
    return socket_create(domain, type, protocol);
}
