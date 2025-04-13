#ifndef PORT_IO_H
#define PORT_IO_H

#include <stdint.h>

uint8_t inb(uint16_t port);
void outb(uint8_t value, uint16_t port);
void outw(uint16_t value, uint16_t port);

#endif