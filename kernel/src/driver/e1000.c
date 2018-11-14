#include "e1000.h"
#include "kprintf.h"
#include "kmalloc.h"
#include "panic.h"
#include "util.h"
#include "arch/x86/vmem.h"
#include "arch/x86/paging.h"
#include <string.h>
#include <stddef.h>


#define E1000_PCI_DEVICE_ID 0x100E

#define REG_CTRL            0x0000
#define REG_EEPROM          0x0014
#define REG_IMASK           0x00D0
#define REG_RCTRL           0x0100
#define REG_RXDESCLO        0x2800
#define REG_RXDESCHI        0x2804
#define REG_RXDESCLEN       0x2808
#define REG_RXDESCHEAD      0x2810
#define REG_RXDESCTAIL      0x2818

#define REG_TCTRL           0x0400
#define REG_TXDESCLO        0x3800
#define REG_TXDESCHI        0x3804
#define REG_TXDESCLEN       0x3808
#define REG_TXDESCHEAD      0x3810
#define REG_TXDESCTAIL      0x3818

#define REG_MTA             0x5200

#define RCTL_EN             (1 << 1)    // Receiver Enable
#define RCTL_SBP            (1 << 2)    // Store Bad Packets
#define RCTL_UPE            (1 << 3)    // Unicast Promiscuous Enabled
#define RCTL_MPE            (1 << 4)    // Multicast Promiscuous Enabled
#define RCTL_LPE            (1 << 5)    // Long Packet Reception Enable
#define RCTL_LBM_NONE       (0 << 6)    // No Loopback
#define RCTL_LBM_PHY        (3 << 6)    // PHY or external SerDesc loopback
#define RTCL_RDMTS_HALF     (0 << 8)    // Free Buffer Threshold is 1/2 of RDLEN
#define RTCL_RDMTS_QUARTER  (1 << 8)    // Free Buffer Threshold is 1/4 of RDLEN
#define RTCL_RDMTS_EIGHTH   (2 << 8)    // Free Buffer Threshold is 1/8 of RDLEN
#define RCTL_MO_36          (0 << 12)   // Multicast Offset - bits 47:36
#define RCTL_MO_35          (1 << 12)   // Multicast Offset - bits 46:35
#define RCTL_MO_34          (2 << 12)   // Multicast Offset - bits 45:34
#define RCTL_MO_32          (3 << 12)   // Multicast Offset - bits 43:32
#define RCTL_BAM            (1 << 15)   // Broadcast Accept Mode
#define RCTL_VFE            (1 << 18)   // VLAN Filter Enable
#define RCTL_CFIEN          (1 << 19)   // Canonical Form Indicator Enable
#define RCTL_CFI            (1 << 20)   // Canonical Form Indicator Bit Value
#define RCTL_DPF            (1 << 22)   // Discard Pause Frames
#define RCTL_PMCF           (1 << 23)   // Pass MAC Control Frames
#define RCTL_SECRC          (1 << 26)   // Strip Ethernet CRC

// Buffer Sizes
#define RCTL_BSIZE_256      (3 << 16)
#define RCTL_BSIZE_512      (2 << 16)
#define RCTL_BSIZE_1024     (1 << 16)
#define RCTL_BSIZE_2048     (0 << 16)
#define RCTL_BSIZE_4096     ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192     ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384    ((1 << 16) | (1 << 25))

#define ECTRL_FD            0x01    //FULL DUPLEX
#define ECTRL_ASDE          0x20    //auto speed enable
#define ECTRL_SLU           0x40    //set link up


// TCTL Register

#define TCTL_EN             (1 << 1)    // Transmit Enable
#define TCTL_PSP            (1 << 3)    // Pad Short Packets
#define TCTL_CT_SHIFT       4           // Collision Threshold
#define TCTL_COLD_SHIFT     12          // Collision Distance
#define TCTL_SWXOFF         (1 << 22)   // Software XOFF Transmission
#define TCTL_RTLC           (1 << 24)   // Re-transmit on Late Collision

#define TSTA_DD             (1 << 0)    // Descriptor Done
#define TSTA_EC             (1 << 1)    // Excess Collisions
#define TSTA_LC             (1 << 2)    // Late Collision
#define LSTA_TU             (1 << 3)    // Transmit Underrun

// Transmit Command
#define CMD_EOP             (1 << 0)    // End of Packet
#define CMD_IFCS            (1 << 1)    // Insert FCS
#define CMD_IC              (1 << 2)    // Insert Checksum
#define CMD_RS              (1 << 3)    // Report Status
#define CMD_RPS             (1 << 4)    // Report Packet Sent
#define CMD_VLE             (1 << 6)    // VLAN Packet Enable
#define CMD_IDE             (1 << 7)    // Interrupt Delay Enable

#define RX_FLAGS (RCTL_EN | RCTL_SBP | RCTL_UPE | RCTL_MPE | RCTL_LBM_NONE \
                | RTCL_RDMTS_HALF | RCTL_BAM | RCTL_SECRC | RCTL_BSIZE_2048)

#define TX_FLAGS (TCTL_EN | TCTL_PSP | \
        (15 << TCTL_CT_SHIFT) | (64 << TCTL_COLD_SHIFT) | TCTL_RTLC)

void e1000_outl(struct e1000 *e, uint16_t addr, uint32_t val)
{
    *(uint32_t *)(e->pci->mm_base + addr) = val;
}

uint32_t e1000_inl(struct e1000 *e, uint16_t addr)
{
    return *(uint32_t *)(e->pci->mm_base + addr);
}


uint32_t e1000_eeprom_read(struct e1000 *e, uint8_t addr)
{
    uint32_t val = 0;
    uint32_t test;

    if(e->is_e)
        test = addr << 8;
    else
        test = addr << 2;
    e1000_outl(e, REG_EEPROM, test | 0x1);
    if(e->is_e)
    {
        while(!((val = e1000_inl(e, REG_EEPROM)) & (1<<4)))
            kprintf("val: %x\n", val);

    }
    else
    {
        while(!((val = e1000_inl(e, REG_EEPROM)) & (1<<1)))
            kprintf("val: %x\n", val);
    }
    val >>= 16;
    return val;
}

void e1000_get_type(struct e1000 *e)
{
    int i;
    uint32_t val = 0;
    e1000_outl(e, REG_EEPROM, 1);
    for (i = 0; i < 1000 && !e->is_e; i++)
    {
        val = e1000_inl(e, REG_EEPROM);
        e->is_e = (val & 0x10) ? 1 : 0;
    }
}

void e1000_get_mac(struct e1000 *e)
{
    uint32_t temp;
    temp = e1000_eeprom_read(e, 0);
    e->mac[0] = temp & 0xff;
    e->mac[1] = temp >> 8;
    temp = e1000_eeprom_read(e, 1);
    e->mac[2] = temp & 0xff;
    e->mac[3] = temp >> 8;
    temp = e1000_eeprom_read(e, 2);
    e->mac[4] = temp & 0xff;
    e->mac[5] = temp >> 8;
}

void e1000_tx_init(struct e1000 *e)
{
    int i;
    struct e1000_tx_desc *descs;

    descs = kmalloc(sizeof(struct e1000_tx_desc) * (NUM_TX_DESC+1), 0);
    if (!descs)
        panic("Allocating TX descs\n");
    descs = (struct e1000_tx_desc *)ALIGN_UP((uintptr_t)descs, 16);
    memset(descs, 0, sizeof(struct e1000_tx_desc) * NUM_TX_DESC);
    kprintf("TX buf @ 0x%p\n", descs);

    e->tx_free = (uint8_t *)descs;
    for(i = 0; i < NUM_TX_DESC; i++) {
        e->tx_descs[i] = &descs[i];
        e->tx_descs[i]->addr = 0;
        e->tx_descs[i]->cmd = 0;
        e->tx_descs[i]->status = TSTA_DD;
    }

    //give the card the pointer to the descriptors
    e1000_outl(e, REG_TXDESCLO, (uint32_t)virt_to_phys(descs));
    e1000_outl(e, REG_TXDESCHI, 0);

    //now setup total length of descriptors
    e1000_outl(e, REG_TXDESCLEN, NUM_TX_DESC * 16);

    //setup numbers
    e1000_outl(e, REG_TXDESCHEAD, 0);
    e1000_outl(e, REG_TXDESCTAIL, 0);
    e->tx_cur = 0;

    // enable transmission
    e1000_outl(e, REG_TCTRL, TX_FLAGS);
}

void e1000_rx_init(struct e1000 *e)
{
    struct e1000_rx_desc *descs;
    void *ptr;
    int i;

    descs = kmalloc(sizeof(struct e1000_rx_desc) * (NUM_RX_DESC+1), 0);
    if (!descs)
        panic("Allocating RX descs\n");
    descs = (struct e1000_rx_desc *)ALIGN_UP((uintptr_t)descs, 16);
    memset(descs, 0, sizeof(struct e1000_rx_desc) * NUM_RX_DESC);
    kprintf("RX buf @ 0x%p\n", descs);

    e->rx_free = (uint8_t *)descs;
    for(i = 0; i < NUM_RX_DESC; i++)
    {
        e->rx_descs[i] = &descs[i];
        ptr = kmalloc(8192+16, 0);
        if (!ptr)
            panic("Out of memory allocating RX buffers");
        e->rx_descs[i]->addr = (uint32_t)virt_to_phys(ptr);
        e->rx_descs[i]->status = 0;
    }

    //give the card the pointer to the descriptors
    e1000_outl(e, REG_RXDESCLO, (uint32_t)virt_to_phys(descs));
    e1000_outl(e, REG_RXDESCHI, 0);

    //now setup total length of descriptors
    e1000_outl(e, REG_RXDESCLEN, NUM_RX_DESC * 16);

    //setup numbers
    e1000_outl(e, REG_RXDESCHEAD, 0);
    e1000_outl(e, REG_RXDESCTAIL, NUM_RX_DESC-1);
    e->rx_cur = 0;

    //enable receiving
    e1000_outl(e, REG_RCTRL, RX_FLAGS);
}

static void raw_to_asc(char *asc, const char *raw, size_t rawsize)
{
    char c;
    int i = rawsize-1;      /* raw index */
    int j = 2*rawsize-1;    /* asc index */

    while (i >= 0)
    {
        c = raw[i] & 0x0F;
        if (c < 10)
            asc[j] = '0' + c;
        else
            asc[j] = 'A' + (c - 10);
        j--;
        c = (raw[i] >> 4) & 0x0F;
        if (c < 10)
            asc[j] = '0' + c;
        else
            asc[j] = 'A' + (c - 10);
        j--;
        i--;
    }
}

size_t e1000_tx(struct e1000 *e, uint8_t *buf, size_t length)
{
    uint8_t old_cur;
    //struct e1000 *e = dev->device;
    char *copy = kmalloc(length, 0);
    if (!copy)
        return 0;
    memcpy(copy, buf, length);
    kprintf("TX buf @ 0x%p\n", buf);

    e->tx_descs[e->tx_cur]->addr = (uintptr_t)virt_to_phys(copy);
    e->tx_descs[e->tx_cur]->length = length;
    e->tx_descs[e->tx_cur]->cmd = CMD_EOP | CMD_IFCS | CMD_RS | CMD_RPS;
    old_cur = e->tx_cur;
    e->tx_cur = (e->tx_cur + 1) % NUM_TX_DESC;
    e1000_outl(e, REG_TXDESCTAIL, e->tx_cur);

    while(!(e->tx_descs[old_cur]->status & 0xff));

    return length;
}

void e1000_rx(struct e1000 *e)
{
    uint16_t old_cur;

    while((e->rx_descs[e->rx_cur]->status & 0x1))
    {
        uint8_t *buf = (uint8_t *)e->rx_descs[e->rx_cur]->addr;
        uint16_t len = e->rx_descs[e->rx_cur]->length;
        char *str;

        str = kmalloc(len*2+1, 0);
        raw_to_asc(str, phys_to_virt(buf), len);
        str[len*2] = '\0';

        kprintf("[len %d] ", len);
        kprintf("%s", str);
        kprintf("\n");
        kfree(str, len*2+1);

        e->rx_descs[e->rx_cur]->status = 0;
        old_cur = e->rx_cur;
        e->rx_cur = (e->rx_cur + 1) % NUM_RX_DESC;
        e1000_outl(e, REG_RXDESCTAIL, old_cur);
    }
}

struct e1000 *g_e = NULL;

void e1000_handler(void)
{
    struct e1000 *e = g_e;
    uint32_t status = e1000_inl(e, 0xc0);

    kprintf("e1000 interrupt : status=%x\n", status);

    // TODO free the tx buffer

    if(status & 0x04)
    {
    //  e1000_linkup(e);
        kprintf("linkup\n");
    }

    if(status & 0x10)
    {
        kprintf("threshold good\n");
    }

    if(status & 0x80)
        e1000_rx(e);
}

int e1000_init(struct e1000 *e)
{
    uint32_t val;

    g_e = e;

    e->pci = pci_get_device(PCI_VENDOR_ID_INTEL, E1000_PCI_DEVICE_ID);
    if (e->pci == NULL)
        return -1;

    page_map((void *)e->pci->mm_base, e->pci->mm_base);
    page_map((void *)e->pci->mm_base + REG_MTA, e->pci->mm_base + REG_MTA);
    page_map((void *)e->pci->mm_base + REG_TXDESCLO, e->pci->mm_base + REG_TXDESCLO);
    page_map((void *)e->pci->mm_base + REG_RXDESCLO, e->pci->mm_base + REG_RXDESCLO);

    e1000_get_type(e);
    e1000_get_mac(e);

    kprintf("Intel Pro/1000 Ethernet adapter found\n");
    kprintf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
            e->mac[0], e->mac[1], e->mac[2], e->mac[3], e->mac[4], e->mac[5]);

    pci_register_handler(e->pci, e1000_handler);

    /* Set the link up */
    val = e1000_inl(e,REG_CTRL);
    e1000_outl(e, REG_CTRL, val | ECTRL_SLU);


    /* Initialize the multicase table array */
    int i;
    for (i = 0; i < 128; i++)
        e1000_outl(e, REG_MTA + (i * 4), 0);

    /* Enable all interrupts and clear existing pending ones */
    e1000_outl(e, REG_IMASK , 0x1F6DC);
    e1000_outl(e, REG_IMASK , 0xff & ~4);
    e1000_inl(e, 0xc0);

    /* CHicken OS */
    e1000_tx_init(e);
    e1000_rx_init(e);

    return 0;
}
