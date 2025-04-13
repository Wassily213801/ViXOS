#include "port_io.h"

void shutdown() {
    outw(0x2000, 0x604); // выключение QEMU через ACPI
}

void reboot() {
    uint8_t temp;
    __asm__ volatile("cli");
    temp = inb(0x64);
    while (temp & 0x02) temp = inb(0x64);
    outb(0xFE, 0x64);
}