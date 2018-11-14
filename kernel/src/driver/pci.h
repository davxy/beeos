#ifndef BEEOS_DRIVER_PCI_H_
#define BEEOS_DRIVER_PCI_H_

#include <stdint.h>
#include <sys/types.h>

#define PCI_VENDOR_ID_INTEL     0x8086

#define PCI_BAR_MEM             0
#define PCI_BAR_IO              1

struct pci_device {
    struct pci_device *next;
    unsigned char   bus;
    unsigned char   slot;
    unsigned short  vendor_id;
    unsigned short  device_id;
    unsigned char   class_code;
    unsigned char   subclass;
    unsigned char   prog_if;
    uintptr_t       io_base;
    uintptr_t       mm_base;
    int             int_line;
    int             int_pin;
};

void pci_init(void);

struct pci_device *pci_get_device(int vendor_id, int device_id);

void pci_register_handler(struct pci_device *dev, void (*handler)(void));


#endif /* BEEOS_DRIVER_PCI_H_ */
