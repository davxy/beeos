/*
 * /dev/eth0 smoke test.
 *
 * Sends an ARP request for the qemu user-mode network gateway
 * (10.0.2.2) and waits for the reply.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define ETH_DEV     "/dev/eth0"
#define MAX_TRIES   10

/*
 * Ethernet + ARP request, network byte order.
 * The source MAC is the qemu default (52:54:00:12:34:56); the reply is
 * received anyway since the interface works in promiscuous mode.
 */
static const uint8_t arp_req[42] = {
    /* ethernet: dst, src, ethertype */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x52, 0x54, 0x00, 0x12, 0x34, 0x56,
    0x08, 0x06,
    /* arp: htype=1, ptype=0x0800, hlen=6, plen=4, op=1 (request) */
    0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01,
    /* sender: 52:54:00:12:34:56, 10.0.2.15 */
    0x52, 0x54, 0x00, 0x12, 0x34, 0x56, 0x0A, 0x00, 0x02, 0x0F,
    /* target: 00:00:00:00:00:00, 10.0.2.2 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x02, 0x02
};

int main(void)
{
    int fd, i;
    ssize_t n;
    uint8_t frame[2048];

    fd = open(ETH_DEV, O_RDWR, 0);
    if (fd < 0) {
        printf("error: cannot open %s\n", ETH_DEV);
        return 1;
    }

    n = write(fd, arp_req, sizeof(arp_req));
    if (n != sizeof(arp_req)) {
        printf("error: arp request write failure (%d)\n", (int)n);
        return 1;
    }
    printf("arp request sent, who has 10.0.2.2?\n");

    for (i = 0; i < MAX_TRIES; i++) {
        n = read(fd, frame, sizeof(frame));
        if (n < 42)
            continue;
        /* ARP reply for 10.0.2.2 ? */
        if (frame[12] == 0x08 && frame[13] == 0x06 &&
            frame[20] == 0x00 && frame[21] == 0x02 &&
            memcmp(&frame[28], "\x0A\x00\x02\x02", 4) == 0) {
            printf("10.0.2.2 is at %02x:%02x:%02x:%02x:%02x:%02x\n",
                   frame[22], frame[23], frame[24],
                   frame[25], frame[26], frame[27]);
            close(fd);
            return 0;
        }
    }

    printf("error: no arp reply\n");
    close(fd);
    return 1;
}
