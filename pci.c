#include "pci.h"
#include "port_io.h"
#include "video.h"

uint32_t pci_read_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address = (1 << 31) |
                      ((uint32_t)bus << 16) |
                      ((uint32_t)device << 11) |
                      ((uint32_t)func << 8) |
                      (offset & 0xFC);
    
    outl(address, PCI_CONFIG_ADDRESS);
    return inl(PCI_CONFIG_DATA);
}

void pci_write_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) |
                      ((uint32_t)bus << 16) |
                      ((uint32_t)device << 11) |
                      ((uint32_t)func << 8) |
                      (offset & 0xFC);
    
    outl(address, PCI_CONFIG_ADDRESS);
    outl(value, PCI_CONFIG_DATA);
}

pci_device_t pci_find_device(uint16_t vendor_id, uint16_t device_id, uint8_t class_code, uint8_t subclass) {
    pci_device_t dev;
    dev.vendor_id = 0xFFFF;
    
    for (int bus = 0; bus < 256; bus++) {
        for (int slot = 0; slot < 32; slot++) {
            for (int func = 0; func < 8; func++) {
                uint32_t data = pci_read_config(bus, slot, func, 0);
                uint16_t found_vendor = data & 0xFFFF;
                uint16_t found_device = data >> 16;
                
                if (found_vendor == 0xFFFF) continue;
                
                // Проверяем класс и подкласс
                data = pci_read_config(bus, slot, func, 8);
                uint8_t found_class = (data >> 24) & 0xFF;
                uint8_t found_subclass = (data >> 16) & 0xFF;
                
                if ((vendor_id == 0xFFFF || found_vendor == vendor_id) &&
                    (device_id == 0xFFFF || found_device == device_id) &&
                    (class_code == 0xFF || found_class == class_code) &&
                    (subclass == 0xFF || found_subclass == subclass)) {
                    
                    dev.vendor_id = found_vendor;
                    dev.device_id = found_device;
                    dev.class_code = found_class;
                    dev.subclass = found_subclass;
                    dev.prog_if = (data >> 8) & 0xFF;
                    dev.revision = data & 0xFF;
                    dev.bus = bus;
                    dev.device = slot;
                    dev.function = func;
                    
                    // Читаем BAR
                    for (int i = 0; i < 6; i++) {
                        dev.bar[i] = pci_read_config(bus, slot, func, 0x10 + i * 4);
                    }
                    
                    return dev;
                }
            }
        }
    }
    
    return dev;
}

void pci_enable_busmaster(pci_device_t dev) {
    uint32_t cmd = pci_read_config(dev.bus, dev.device, dev.function, 0x04);
    cmd |= (1 << 2);  // Bus Master Enable
    pci_write_config(dev.bus, dev.device, dev.function, 0x04, cmd);
}

void pci_enable_memory_space(pci_device_t dev) {
    uint32_t cmd = pci_read_config(dev.bus, dev.device, dev.function, 0x04);
    cmd |= (1 << 1);  // Memory Space Enable
    pci_write_config(dev.bus, dev.device, dev.function, 0x04, cmd);
}

uint32_t pci_get_bar(pci_device_t dev, int bar_num) {
    if (bar_num < 0 || bar_num >= 6) return 0;
    
    uint32_t bar = dev.bar[bar_num];
    if (bar & 1) {
        // I/O Space BAR
        return bar & ~3;
    } else {
        // Memory Space BAR
        return bar & ~0xF;
    }
}