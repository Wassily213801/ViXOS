#ifndef PORT_IO_H
#define PORT_IO_H

#include <stdint.h>

uint8_t inb(uint16_t port);
void outb(uint8_t value, uint16_t port);
void outw(uint16_t value, uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);
void outl(uint32_t value, uint16_t port);

void pc_speaker_play(uint32_t frequency);
void pc_speaker_beep(uint32_t frequency, uint32_t duration_ms);
void pc_speaker_stop(void);

#endif