#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t revision;
    uint32_t bar[6];
    uint8_t bus;
    uint8_t device;
    uint8_t function;
} pci_device_t;

// PCI Class Codes for AHCI
#define PCI_CLASS_STORAGE    0x01
#define PCI_SUBCLASS_SATA    0x06
#define PCI_PROGIF_AHCI      0x01

uint32_t pci_read_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
void pci_write_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t value);
pci_device_t pci_find_device(uint16_t vendor_id, uint16_t device_id, uint8_t class_code, uint8_t subclass);
void pci_enable_busmaster(pci_device_t dev);
void pci_enable_memory_space(pci_device_t dev);
uint32_t pci_get_bar(pci_device_t dev, int bar_num);

#endif