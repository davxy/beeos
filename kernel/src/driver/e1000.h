#ifndef BEEOS_DRIVER_E1000_H_
#define BEEOS_DRIVER_E1000_H_

#include "pci.h"


#define NUM_RX_DESC 16 //256
#define NUM_TX_DESC 16 //256


struct e1000_rx_desc {
    uint64_t addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} __attribute__((packed));

struct e1000_tx_desc {
    uint64_t addr;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} __attribute__((packed));


struct e1000 {
    struct pci_device *pci;
    int     is_e;
    uint8_t mac[6];
    uint16_t rx_cur;
    uint16_t tx_cur;
    uint8_t *rx_free;
    uint8_t *tx_free;
    struct e1000_rx_desc *rx_descs[NUM_RX_DESC];
    struct e1000_tx_desc *tx_descs[NUM_TX_DESC];
};

int e1000_init(struct e1000 *e);


#endif /* BEEOS_DRIVER_E1000_H_ */
