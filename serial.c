#include "serial.h"
#include "port_io.h"

#define SERIAL_PORT_COM1 0x3F8

void serial_init(void) {
    // Disable interrupts
    outb(0x00, SERIAL_PORT_COM1 + 1);
    // Enable DLAB (set baud rate divisor)
    outb(0x80, SERIAL_PORT_COM1 + 1);
    // Set divisor to 3 (lo byte) 38400 baud
    outb(0x03, SERIAL_PORT_COM1 + 0);
    //                  (hi byte)
    outb(0x00, SERIAL_PORT_COM1 + 1);
    // 8 bits, no parity, one stop bit
    outb(0x03, SERIAL_PORT_COM1 + 3);
    // Enable FIFO, clear them, with 14-byte threshold
    outb(0xC7, SERIAL_PORT_COM1 + 2);
    // IRQs enabled, RTS/DSR set
    outb(0x0B, SERIAL_PORT_COM1 + 4);
}

static int serial_is_transmit_empty(void) {
    return inb(SERIAL_PORT_COM1 + 5) & 0x20;
}

void serial_write_char(char c) {
    while (!serial_is_transmit_empty()) {
        // wait
    }
    outb((uint8_t)c, SERIAL_PORT_COM1 + 0);
}

void serial_write(const char* s) {
    while (*s) {
        if (*s == '\n') serial_write_char('\r');
        serial_write_char(*s++);
    }
}
