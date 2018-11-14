#include "pci.h"
#include "kmalloc.h"
#include "panic.h"
#include "arch/x86/io.h"
#include "arch/x86/isr_arch.h"
#include "kprintf.h"
#include "isr.h"

struct pci_conf_hdr
{
    uint16_t    vendor_id;
    uint16_t    device_id;
    uint16_t    command;
    uint16_t    status;
    uint8_t     revision;
    uint8_t     pci_interface;
    uint8_t     pci_minor;
    uint8_t     pci_major;
    uint8_t     cache_line_siz;
    uint8_t     latency;
    uint8_t     header_type;
    uint8_t     bist;
    uint32_t    bars[6];
    uint32_t    cardbus_cis_pointer;
    uint32_t    expanion_rom_base_addr;
    uint16_t    subsystem_vendor_id;
    uint16_t    subsystem_id;
    uint32_t    cabapbilities_pointer;
    uint32_t    reserved;
    uint8_t     int_line;
    uint8_t     int_pin;
    uint8_t     min_grant;
    uint8_t     max_lat;
};

union cfg_addr
{
    struct
    {
        unsigned type : 2;
        unsigned reg : 6;
        unsigned function : 3;
        unsigned slot : 5;
        unsigned bus : 8;
        unsigned res : 7;
        unsigned enable : 1;
    };
    uint32_t val;
};

struct pci_device *pci_device_list;

#define PCI_ADDR_PORT   0x0CF8
#define PCI_DATA_PORT   0x0CFC

#define PCI_MAX_BUSES   256
#define PCI_MAX_SLOTS   32


struct pci_irq
{
    void    (*handler)();
    int     int_pin;
    void    *aux;
};

struct pci_irq pci_irqs[32][4];


struct pci_device *pci_get_device(int vendor_id, int device_id)
{
    struct pci_device *dev;
    for (dev = pci_device_list; dev != NULL; dev = dev->next)
    {
        if (dev->vendor_id == vendor_id && dev->device_id == device_id)
            return dev;
    }
    return NULL;
}

#include "proc.h"

void pci_handler(void)
{
    int i;
    int int_no = current->arch.ifr->int_no - 32;

    for(i = 0; i < 4; i++) {
        if(pci_irqs[int_no][i].handler != NULL)
            pci_irqs[int_no][i].handler();
    }
}

void pci_register_handler(struct pci_device *dev, void (*handler)(void))
{
    struct pci_irq *irq;
    int int_line = dev->int_line;
    int int_pin  = dev->int_pin;

    irq = &pci_irqs[int_line][int_pin];
    irq->int_pin = int_pin;
    irq->handler = handler;
    kprintf("Register PCI int on int-line %d (ISR%d)\n",
            int_line, ISR_IRQ0 + int_line);
    isr_register_handler(ISR_IRQ0 + int_line, &pci_handler);
}

void pci_init(void)
{
    int b, s;
    uint32_t *p32;
    struct pci_conf_hdr hdr;
    union cfg_addr addr;
    struct pci_device *pcidev;

    addr.val = 0;
    addr.enable = 1;
    p32 = (uint32_t *)&hdr;
    for (b = 0; b < PCI_MAX_BUSES; b++)
    {
        for (s = 0; s < PCI_MAX_SLOTS; s++)
        {
            addr.bus = b;
            addr.slot = s;
            addr.reg = 0;
            outl(PCI_ADDR_PORT, addr.val);
            p32[0] = inl(PCI_DATA_PORT);
            if (p32[0] == 0xFFFFFFFF)
                continue; /* There is no such bus:slot */
            for (addr.reg = 1; addr.reg < 16; addr.reg++)
            {
                outl(PCI_ADDR_PORT, addr.val);
                p32[addr.reg] = inl(PCI_DATA_PORT);
            }

            /*
             * We consider PCI header type 0x00 and we do not consider
             * PCI bridge devices
             */
            if (hdr.header_type != 0x00 || hdr.pci_major == 0x06)
                continue;

            pcidev = kmalloc(sizeof(struct pci_device), 0);
            if (!pcidev)
                panic("No memory for pci device");
            pcidev->vendor_id = hdr.vendor_id;
            pcidev->device_id = hdr.device_id;
            pcidev->mm_base = hdr.bars[0] & ~3;
            pcidev->io_base = hdr.bars[1] & 0xFFFFFFFC; // depends on device
            pcidev->int_line = hdr.int_line;
            pcidev->int_pin = hdr.int_pin;
            pcidev->next = pci_device_list;
            pci_device_list = pcidev;

            /* enable PCI bus mastering. TODO: create a function for this */
            if (pcidev->vendor_id == 0x8086)
            {
                hdr.command |= (1 << 2);
                addr.bus = b;
                addr.slot = s;
                addr.reg = 0;
                for (addr.reg = 0; addr.reg < 16; addr.reg++)
                {
                    outl(PCI_ADDR_PORT, addr.val);
                    outl(PCI_DATA_PORT, p32[addr.reg]);
                }
            }
            kprintf("pcidev detected. Vendor: 0x%04x, Device: 0x%04x\n",
                    pcidev->vendor_id, pcidev->device_id);
        }
    }
}
