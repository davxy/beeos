#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

static char *pdu = "FFFFFFFFFFFFA32191C362D408060001080006040001A22191C362D4C0A80002FFFFFFFFFFFFC0A80002000000000000000000000000000000000000";

static void asc_to_raw(char *raw, const char *asc, size_t ascsize)
{
    char c, r;
    int i = 0, j = 0;
    while (i < ascsize) {
        c = asc[i];
        if ('0' <= c && c <= '9')
            r = (c - '0');
        else if ('a' <= c && c <= 'f')
            r = (c - 'a') + 10;
        else if ('A' <= c && c <= 'F')
            r = (c - 'A') + 10;
        if (i % 2 == 0)
            raw[j] = (r << 4);
        else
            raw[j++] |= r;
        i++;
    }
}

int main(int argc, char *argv[])
{
    int fd;
    char *buf;
    size_t pdulen = strlen(pdu)>>1;
    int n = 1;

    if (argc > 1)
        n = atol(argv[1]);

    buf = malloc(pdulen);
    if (buf == NULL)
        err_sys("malloc error");
    asc_to_raw(buf, pdu, strlen(pdu));

    if ((fd = socket(AF_PACKET, SOCK_RAW, 0)) < 0)
        err_sys("socket error");

    while (n-- > 0) {
        if (write(fd, buf, pdulen) != pdulen)
            err_sys("write error");
        sleep(1);
    }
    return 0;
}
