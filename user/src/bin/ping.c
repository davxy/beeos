#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/icmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

struct ip_pkt {
  struct iphdr ip;      /* <netinet/ip.h> */
  struct icmphdr icmp;  /* <netinet/icmp.h> */
  char data[60];
} __attribute__ ((packed));

struct icmp_pkt {
  struct icmphdr icmp;  /* <netinet/icmp.h> */
  char data[60];
} __attribute__ ((packed));

static uint16_t ip_chk(uint16_t * data, size_t size)
{
    int i;
    uint32_t sum;
    uint16_t carry;
    uint16_t checksum;
    uint16_t last;
    uint8_t *octet = (uint8_t *) data;

    for (i = 0, sum = 0; i < (size / 2); i++)
        sum += ntohs (data[i]);
    if (size % 2) {
        /* The size is odd, and the last octet must be accounted too. */
        last = octet[size - 1];
        last = last << 8;
        sum += last;
      }
    /* Extract the carries and make the checksum. */
    carry = sum >> 16;
    checksum = sum & 0x0000FFFF;
    checksum += carry;
    return checksum;
}

int main(int argc, char *argv[])
{
    int sfdn;
    struct sockaddr_in sa;
    ssize_t sent;
    ssize_t received;
    int status;
    char *destination;
    uint16_t checksum;
    struct icmp_pkt icmp_pkt_send;
    struct ip_pkt ip_pkt_receive;
    clock_t clock_ping;
    clock_t clock_pong;
    clock_t clock_time;
  
    if (argc < 2) {
        fprintf (stderr, "Usage: ping <ipv4 addr>\n");
        return 1;
    }
    destination = argv[1];

    sa.sin_family = AF_INET;
    sa.sin_port = 0;
    inet_pton(AF_INET, destination, &sa.sin_addr.s_addr);

    /* Put some data inside the packet header */
    icmp_pkt_send.icmp.un.echo.id = (uint16_t) rand();
    icmp_pkt_send.icmp.un.echo.sequence = 0;
    icmp_pkt_send.icmp.type = 8;  /* Echo request. */
    icmp_pkt_send.icmp.code = 0;
    icmp_pkt_send.icmp.checksum = 0;
    /* Calculate the ICMP checksum */
    checksum = ~(ip_chk((void *) &icmp_pkt_send, sizeof(struct icmp_pkt)));
    icmp_pkt_send.icmp.checksum = htons(checksum);

    /* Open the socket */
    sfdn = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sfdn < 0) {
        perror(NULL);
        return 1;
    }

    /* Connect the `sa' destination */
    status = connect(sfdn, (struct sockaddr *) &sa, sizeof(sa));
    if (status < 0) {
        perror(NULL);
        close(sfdn);
        return 1;
    }

    /*
     * Send one single packet: please notice that we send an ICMP,
     * but we receive a full IP.
     */
    clock_ping = clock();
    sent = send(sfdn, &icmp_pkt_send, sizeof(struct icmp_pkt), 0);
    if (sent < 0) {
        perror(NULL);
        return 1;
    }

    /* Packet sent */
    printf ("ping: ");

    /* Receive all ICMP raw packets, and select the one with the same id */
    while (1) {
        received = read(sfdn, &ip_pkt_receive, sizeof(struct ip_pkt));
        clock_pong = clock ();

        if (ip_pkt_receive.icmp.un.echo.id == icmp_pkt_send.icmp.un.echo.id) {
            clock_time = (clock_pong - clock_ping);
            printf ("pong %llu.%03llu s\n",
                    clock_time / CLOCKS_PER_SEC,
                    (clock_time % CLOCKS_PER_SEC) *
                    1000 / CLOCKS_PER_SEC);
            break;
        }
    }

    close (sfdn);
    return (0);
}
