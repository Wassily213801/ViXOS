#include "port_io.h"

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outb(uint8_t value, uint16_t port) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void outw(uint16_t value, uint16_t port) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}