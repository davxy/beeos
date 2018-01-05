#include <stdint.h>

/*
 * Prime Cell UART (PL011) registers.
 */
struct pl011
{
    uint32_t    data;       /* Data */
    uint32_t    rs_ec;      /* Receive status register / error clear */
    uint32_t    rsvd1[4];   /* Reserved */
    uint32_t    flags;      /* Flags */
    uint32_t    rsvd2;      /* Reserved */
    uint32_t    lpr;        /* IrDA low-power couter */
    uint32_t    ibrd;       /* Integer boud rate */
    uint32_t    fbrd;       /* Fractional boud rate */
    uint32_t    lctrl;      /* Line control register */
    uint32_t    ctrl;       /* Control register */
    uint32_t    ifls;       /* Interrupt FIFO level select */
    uint32_t    imsc;       /* Interrupt mask sec/clear */
    uint32_t    ris;        /* Raw interrupt status */
    uint32_t    mis;        /* Masked interrupt status */
    uint32_t    ic;         /* Interrupt clear */
    uint32_t    dmac;       /* DMA control */
};

/* Flags register bits */
#define CTS     (1 << 0)    /* Clear to send */
#define DSR     (1 << 1)    /* Data set ready */
#define DCD     (1 << 2)    /* Data carrier detect */
#define BUSY    (1 << 3)    /* UART busy. If set the UART is transmitting */
#define RXFE    (1 << 4)    /* Receive FIFO empty */
#define TXFF    (1 << 5)    /* Transmit FIFO full */
#define RXFF    (1 << 6)    /* Receive FIFO full */
#define TXFE    (1 << 7)    /* Transmit FIFO empty */
#define RI      (1 << 8)    /* Ring indicator */

#define UART0   ((struct pl011 *)0x101f1000)
#define UART1   ((struct pl011 *)0x101f2000)
#define UART2   ((struct pl011 *)0x101f3000)

void uart_init(void)
{
}


int uart_getchar(void)
{
    return 0; /* EOF */
}


void uart_putchar(int c)
{
    while (UART0->flags & TXFF);
    UART0->data = c;
}
