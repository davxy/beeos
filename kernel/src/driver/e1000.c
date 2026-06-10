/*
 * Copyright (c) 2015-2018, Davide Galassi. All rights reserved.
 *
 * This file is part of the BeeOS software.
 *
 * BeeOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BeeOS; if not, see <http://www.gnu/licenses/>.
 */

/*
 * Intel PRO/1000 (82540EM) ethernet driver.
 *
 * Reception is interrupt driven: the ISR just wakes up the processes
 * blocked in e1000_read(), the descriptor ring is consumed in process
 * context. Transmission is synchronous: e1000_write() copies the frame
 * in a per-descriptor buffer and the descriptor "done" bit is polled
 * before the descriptor is reused.
 *
 * DMA memory (descriptor rings and packet buffers) is taken from the
 * ZONE_LOW zone. Low memory frames are the only ones covered by the
 * boot-time identity (offset) mapping, thus the only ones for which the
 * phys_to_virt/virt_to_phys relation is guaranteed to hold (high memory
 * pages are lazily backed by arbitrary frames in the page fault handler).
 */

#include "e1000.h"
#include "pci.h"
#include "kprintf.h"
#include "sync/cond.h"
#include "sync/spinlock.h"
#include "mm/frame.h"
#include "mm/zone.h"
#include "arch/x86/vmem.h"
#include "arch/x86/paging.h"
#include "arch/x86/paging_bits.h"
#include <string.h>
#include <stdint.h>
#include <errno.h>


#define E1000_PCI_DEVICE_ID 0x100E      /* 82540EM, qemu default */

/* MMIO registers window size (BAR0) */
#define E1000_MMIO_SIZE     0x20000

#define NUM_RX_DESC         16
#define NUM_TX_DESC         16
#define BUF_SIZE            2048
/* NUM_xX_DESC * BUF_SIZE = 32KB = PAGE_SIZE << 3 */
#define BUF_BLOCK_ORDER     3

/* Registers */
#define REG_CTRL            0x0000      /* Device control */
#define REG_EEPROM          0x0014      /* EEPROM read (EERD) */
#define REG_ICR             0x00C0      /* Interrupt cause read (r/clear) */
#define REG_IMS             0x00D0      /* Interrupt mask set */
#define REG_RCTL            0x0100      /* Receive control */
#define REG_TCTL            0x0400      /* Transmit control */
#define REG_TIPG            0x0410      /* Transmit inter packet gap */
#define REG_RDBAL           0x2800      /* RX descriptor base low */
#define REG_RDBAH           0x2804      /* RX descriptor base high */
#define REG_RDLEN           0x2808      /* RX descriptor ring length */
#define REG_RDH             0x2810      /* RX descriptor head */
#define REG_RDT             0x2818      /* RX descriptor tail */
#define REG_TDBAL           0x3800      /* TX descriptor base low */
#define REG_TDBAH           0x3804      /* TX descriptor base high */
#define REG_TDLEN           0x3808      /* TX descriptor ring length */
#define REG_TDH             0x3810      /* TX descriptor head */
#define REG_TDT             0x3818      /* TX descriptor tail */
#define REG_MTA             0x5200      /* Multicast table array (128 regs) */
#define REG_RAL             0x5400      /* Receive address low (RAL0) */
#define REG_RAH             0x5404      /* Receive address high (RAH0) */

/* Device control register bits */
#define CTRL_SLU            (1 << 6)    /* Set link up */
#define CTRL_RST            (1 << 26)   /* Device reset (self clearing) */

/* Interrupt bits (ICR and IMS) */
#define INT_LSC             (1 << 2)    /* Link status change */
#define INT_RXSEQ           (1 << 3)    /* Receive sequence error */
#define INT_RXDMT0          (1 << 4)    /* RX descriptor minimum threshold */
#define INT_RXO             (1 << 6)    /* Receiver FIFO overrun */
#define INT_RXT0            (1 << 7)    /* Receiver timer interrupt */

/* Receive control register bits */
#define RCTL_EN             (1 << 1)    /* Receiver enable */
#define RCTL_UPE            (1 << 3)    /* Unicast promiscuous enable */
#define RCTL_MPE            (1 << 4)    /* Multicast promiscuous enable */
#define RCTL_LBM_NONE       (0 << 6)    /* No loopback */
#define RCTL_RDMTS_HALF     (0 << 8)    /* Free desc threshold: RDLEN/2 */
#define RCTL_BAM            (1 << 15)   /* Broadcast accept mode */
#define RCTL_BSIZE_2048     (0 << 16)   /* Receive buffer size */
#define RCTL_SECRC          (1 << 26)   /* Strip ethernet CRC */

/* Transmit control register bits */
#define TCTL_EN             (1 << 1)    /* Transmit enable */
#define TCTL_PSP            (1 << 3)    /* Pad short packets */
#define TCTL_CT_SHIFT       4           /* Collision threshold */
#define TCTL_COLD_SHIFT     12          /* Collision distance */
#define TCTL_RTLC           (1 << 24)   /* Re-transmit on late collision */

/* Receive address high register bits */
#define RAH_AV              (1u << 31)  /* Address valid */

/* Receive descriptor status bits */
#define RXD_STAT_DD         (1 << 0)    /* Descriptor done */

/* Transmit descriptor status bits */
#define TXD_STAT_DD         (1 << 0)    /* Descriptor done */

/* Transmit descriptor command bits */
#define TXD_CMD_EOP         (1 << 0)    /* End of packet */
#define TXD_CMD_IFCS        (1 << 1)    /* Insert FCS (ethernet CRC) */
#define TXD_CMD_RS          (1 << 3)    /* Report status (DD write-back) */

#define RCTL_FLAGS (RCTL_EN | RCTL_UPE | RCTL_MPE | RCTL_LBM_NONE \
                  | RCTL_RDMTS_HALF | RCTL_BAM | RCTL_BSIZE_2048 | RCTL_SECRC)

#define TCTL_FLAGS (TCTL_EN | TCTL_PSP | (15 << TCTL_CT_SHIFT) \
                  | (64 << TCTL_COLD_SHIFT) | TCTL_RTLC)

#define IMS_FLAGS (INT_LSC | INT_RXSEQ | INT_RXDMT0 | INT_RXO | INT_RXT0)

/* IEEE 802.3 standard inter packet gap values (IPGT=10, IPGR1=8, IPGR2=6) */
#define TIPG_VALUE (10 | (8 << 10) | (6 << 20))


struct e1000_rx_desc {
    uint64_t addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t  status;
    uint8_t  errors;
    uint16_t special;
} __attribute__((packed));

struct e1000_tx_desc {
    uint64_t addr;
    uint16_t length;
    uint8_t  cso;
    uint8_t  cmd;
    uint8_t  status;
    uint8_t  css;
    uint16_t special;
} __attribute__((packed));


struct e1000 {
    struct pci_device *pci;
    int          eeprom;        /* EEPROM present flag */
    uint8_t      mac[6];
    volatile struct e1000_rx_desc *rx_ring;
    volatile struct e1000_tx_desc *tx_ring;
    uint8_t     *rx_buf;        /* NUM_RX_DESC contiguous BUF_SIZE buffers */
    uint8_t     *tx_buf;        /* NUM_TX_DESC contiguous BUF_SIZE buffers */
    unsigned int rx_cur;        /* First descriptor to consume */
    unsigned int tx_cur;        /* Next descriptor to fill */
    struct cond  rx_cond;       /* Readers wait here for frames */
    struct spinlock tx_lock;
};

static struct e1000 eth;


static void wr32(uint16_t reg, uint32_t val)
{
    *(volatile uint32_t *)(eth.pci->mm_base + reg) = val;
}

static uint32_t rd32(uint16_t reg)
{
    return *(volatile uint32_t *)(eth.pci->mm_base + reg);
}

/*
 * The receive ring is shared with the ISR: every spot where it is
 * accessed from process context must run with interrupts disabled,
 * otherwise the ISR may fire while the rx_cond lock is held and
 * deadlock on it (the handler runs through an interrupt gate, that
 * is with IF=0, so it would spin on the lock forever).
 */
static uint32_t irq_save(void)
{
    uint32_t flags;

    asm volatile("pushfd\n\t"
                 "pop %0\n\t"
                 "cli"
                 : "=r"(flags) : : "memory");
    return flags;
}

static void irq_restore(uint32_t flags)
{
    asm volatile("push %0\n\t"
                 "popfd"
                 : : "r"(flags) : "memory", "cc");
}


static int eeprom_detect(void)
{
    int i;

    wr32(REG_EEPROM, 0x01);
    for (i = 0; i < 1000; i++) {
        if ((rd32(REG_EEPROM) & 0x10) != 0)
            return 1;
    }
    return 0;
}

static uint16_t eeprom_read(uint8_t addr)
{
    uint32_t val;

    wr32(REG_EEPROM, ((uint32_t)addr << 8) | 0x01);
    while (((val = rd32(REG_EEPROM)) & (1 << 4)) == 0)
        ;
    return (uint16_t)(val >> 16);
}

static void mac_read(void)
{
    uint16_t word;
    uint32_t val;
    int i;

    if (eth.eeprom != 0) {
        for (i = 0; i < 3; i++) {
            word = eeprom_read(i);
            eth.mac[i*2] = word & 0xFF;
            eth.mac[i*2 + 1] = word >> 8;
        }
    } else {
        /* No EEPROM: the address has been loaded in RAL0/RAH0 */
        val = rd32(REG_RAL);
        eth.mac[0] = val;
        eth.mac[1] = val >> 8;
        eth.mac[2] = val >> 16;
        eth.mac[3] = val >> 24;
        val = rd32(REG_RAH);
        eth.mac[4] = val;
        eth.mac[5] = val >> 8;
    }
}


static int rx_init(void)
{
    int i;
    void *ring_phys, *buf_phys;

    ring_phys = frame_alloc(0, ZONE_LOW);
    if (ring_phys == NULL)
        return -1;
    buf_phys = frame_alloc(BUF_BLOCK_ORDER, ZONE_LOW);
    if (buf_phys == NULL) {
        frame_free(ring_phys, 0);
        return -1;
    }

    eth.rx_ring = (struct e1000_rx_desc *)phys_to_virt(ring_phys);
    eth.rx_buf = (uint8_t *)phys_to_virt(buf_phys);
    eth.rx_cur = 0;

    memset((void *)eth.rx_ring, 0, NUM_RX_DESC * sizeof(*eth.rx_ring));
    for (i = 0; i < NUM_RX_DESC; i++)
        eth.rx_ring[i].addr = (uint32_t)buf_phys + i*BUF_SIZE;

    wr32(REG_RDBAL, (uint32_t)ring_phys);
    wr32(REG_RDBAH, 0);
    wr32(REG_RDLEN, NUM_RX_DESC * sizeof(*eth.rx_ring));
    wr32(REG_RDH, 0);
    wr32(REG_RDT, NUM_RX_DESC - 1);
    wr32(REG_RCTL, RCTL_FLAGS);
    return 0;
}

static int tx_init(void)
{
    int i;
    void *ring_phys, *buf_phys;

    ring_phys = frame_alloc(0, ZONE_LOW);
    if (ring_phys == NULL)
        return -1;
    buf_phys = frame_alloc(BUF_BLOCK_ORDER, ZONE_LOW);
    if (buf_phys == NULL) {
        frame_free(ring_phys, 0);
        return -1;
    }

    eth.tx_ring = (struct e1000_tx_desc *)phys_to_virt(ring_phys);
    eth.tx_buf = (uint8_t *)phys_to_virt(buf_phys);
    eth.tx_cur = 0;

    memset((void *)eth.tx_ring, 0, NUM_TX_DESC * sizeof(*eth.tx_ring));
    for (i = 0; i < NUM_TX_DESC; i++) {
        eth.tx_ring[i].addr = (uint32_t)buf_phys + i*BUF_SIZE;
        /* Mark as completed so that the first use does not wait */
        eth.tx_ring[i].status = TXD_STAT_DD;
    }

    wr32(REG_TDBAL, (uint32_t)ring_phys);
    wr32(REG_TDBAH, 0);
    wr32(REG_TDLEN, NUM_TX_DESC * sizeof(*eth.tx_ring));
    wr32(REG_TDH, 0);
    wr32(REG_TDT, 0);
    wr32(REG_TIPG, TIPG_VALUE);
    wr32(REG_TCTL, TCTL_FLAGS);
    return 0;
}


ssize_t e1000_read(void *buf, size_t size)
{
    unsigned int i;
    size_t n;
    uint32_t flags;

    if (eth.pci == NULL)
        return -ENODEV;

    flags = irq_save();
    spinlock_lock(&eth.rx_cond.lock);

    while ((eth.rx_ring[eth.rx_cur].status & RXD_STAT_DD) == 0) {
        cond_wait(&eth.rx_cond);
        /* The interrupt flag state is unpredictable after the switch */
        (void)irq_save();
    }

    i = eth.rx_cur;
    n = eth.rx_ring[i].length;
    if (n > size)
        n = size;
    memcpy(buf, eth.rx_buf + i*BUF_SIZE, n);
    eth.rx_ring[i].status = 0;
    eth.rx_cur = (i + 1) % NUM_RX_DESC;
    wr32(REG_RDT, i);   /* Give the descriptor back to the hardware */

    spinlock_unlock(&eth.rx_cond.lock);
    irq_restore(flags);
    return (ssize_t)n;
}

ssize_t e1000_write(const void *buf, size_t size)
{
    volatile struct e1000_tx_desc *desc;

    if (eth.pci == NULL)
        return -ENODEV;
    if (size == 0)
        return 0;
    if (size > BUF_SIZE)
        return -EINVAL;

    spinlock_lock(&eth.tx_lock);

    desc = &eth.tx_ring[eth.tx_cur];
    /* Wait for the descriptor (and its buffer) release by the hardware */
    while ((desc->status & TXD_STAT_DD) == 0)
        ;
    memcpy(eth.tx_buf + eth.tx_cur*BUF_SIZE, buf, size);
    desc->length = size;
    desc->status = 0;
    desc->cmd = TXD_CMD_EOP | TXD_CMD_IFCS | TXD_CMD_RS;
    eth.tx_cur = (eth.tx_cur + 1) % NUM_TX_DESC;
    wr32(REG_TDT, eth.tx_cur);

    spinlock_unlock(&eth.tx_lock);
    return (ssize_t)size;
}


static void e1000_isr(void)
{
    uint32_t icr;

    icr = rd32(REG_ICR);    /* Reading also clears the pending causes */
    if (icr == 0)
        return;     /* Shared interrupt line, not for us */

    if ((icr & INT_LSC) != 0)
        wr32(REG_CTRL, rd32(REG_CTRL) | CTRL_SLU);

    /*
     * No lock around the wakeup: the cond queue is manipulated by the
     * readers with interrupts disabled, so the ISR can never observe
     * it in an inconsistent state.
     */
    if ((icr & (INT_RXT0 | INT_RXDMT0 | INT_RXO)) != 0)
        cond_broadcast(&eth.rx_cond);
}


int e1000_init(void)
{
    int i;
    uint32_t addr;

    eth.pci = pci_get_device(PCI_VENDOR_ID_INTEL, E1000_PCI_DEVICE_ID);
    if (eth.pci == NULL || eth.pci->mm_base == 0) {
        eth.pci = NULL;
        return -1;
    }

    pci_bus_master_enable(eth.pci);

    /* Identity map the MMIO registers window */
    for (addr = 0; addr < E1000_MMIO_SIZE; addr += PAGE_SIZE)
        page_map((void *)(eth.pci->mm_base + addr), eth.pci->mm_base + addr);

    /* Reset the device and wait for completion */
    wr32(REG_CTRL, rd32(REG_CTRL) | CTRL_RST);
    for (i = 0; i < 100000; i++) {
        if ((rd32(REG_CTRL) & CTRL_RST) == 0)
            break;
    }
    if ((rd32(REG_CTRL) & CTRL_RST) != 0) {
        kprintf("e1000: reset failure\n");
        eth.pci = NULL;
        return -1;
    }

    wr32(REG_CTRL, rd32(REG_CTRL) | CTRL_SLU);

    eth.eeprom = eeprom_detect();
    mac_read();

    /* Receive address 0: perfect match filter for our MAC */
    wr32(REG_RAL, (uint32_t)eth.mac[0] | ((uint32_t)eth.mac[1] << 8) |
            ((uint32_t)eth.mac[2] << 16) | ((uint32_t)eth.mac[3] << 24));
    wr32(REG_RAH, (uint32_t)eth.mac[4] | ((uint32_t)eth.mac[5] << 8) |
            RAH_AV);

    /* Clear the multicast table array */
    for (i = 0; i < 128; i++)
        wr32(REG_MTA + i*4, 0);

    cond_init(&eth.rx_cond);
    spinlock_init(&eth.tx_lock);

    if (rx_init() < 0 || tx_init() < 0) {
        kprintf("e1000: out of low memory for DMA buffers\n");
        eth.pci = NULL;
        return -1;
    }

    pci_register_handler(eth.pci, e1000_isr);

    /* Enable the interesting interrupts and clear the pending ones */
    wr32(REG_IMS, IMS_FLAGS);
    (void)rd32(REG_ICR);

    kprintf("Intel PRO/1000 ethernet adapter (irq=%d)\n", eth.pci->int_line);
    kprintf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
            eth.mac[0], eth.mac[1], eth.mac[2],
            eth.mac[3], eth.mac[4], eth.mac[5]);
    return 0;
}
