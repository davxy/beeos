#include "sys.h"

int socket_create(int, int, int);

int sys_socket(int domain, int type, int protocol)
{
    return socket_create(domain, type, protocol);
}
