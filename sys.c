// sys.c
#include "port_io.h"
#include "video.h"
#include "keyboard.h"
#include "shutdown_screen.h"

void shutdown() {
    outw(0x2000, 0x604);
    outw(0x2000, 0x4004);
}

void reboot() {
    uint8_t temp;
    __asm__ volatile("cli");
    temp = inb(0x64);
    while (temp & 0x02) temp = inb(0x64);
    outb(0xFE, 0x64);
}