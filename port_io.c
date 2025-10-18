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

uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outl(uint32_t value, uint16_t port) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

// PC Speaker specific functions
void pc_speaker_play(uint32_t frequency) {
    if (frequency == 0) {
        outb(inb(0x61) & 0xFC, 0x61); // Turn speaker off
        return;
    }

    uint32_t divisor = 1193180 / frequency;
    outb(0xB6, 0x43);                 // Set command byte
    outb((uint8_t)divisor, 0x42);     // Set low byte of divisor
    outb((uint8_t)(divisor >> 8), 0x42); // Set high byte of divisor
    
    // Turn speaker on
    uint8_t tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(tmp | 3, 0x61);
    }
}

void pc_speaker_beep(uint32_t frequency, uint32_t duration_ms) {
    pc_speaker_play(frequency);
    // Note: timer_wait function should be implemented elsewhere
    // For now, we'll just play the sound without duration control
}

void pc_speaker_stop() {
    pc_speaker_play(0);
}