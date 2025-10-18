#ifndef PORT_IO_AUDIO_H
#define PORT_IO_AUDIO_H

#include <stdint.h>

// Базовые операции ввода-вывода
uint8_t audio_inb(uint16_t port);
void audio_outb(uint8_t value, uint16_t port);
uint16_t audio_inw(uint16_t port);
void audio_outw(uint16_t value, uint16_t port);
uint32_t audio_inl(uint16_t port);
void audio_outl(uint32_t value, uint16_t port);

// Оптимизированные операции для аудиоустройств
void audio_delay(uint32_t microseconds);
uint8_t audio_wait_for_status(uint16_t port, uint8_t mask, uint8_t expected, uint32_t timeout_us);
void audio_write_sequence(uint16_t base_port, const uint8_t* data, uint32_t length);

// Макросы для быстрого доступа к портам
#define AUDIO_INB(port) audio_inb(port)
#define AUDIO_OUTB(value, port) audio_outb(value, port)
#define AUDIO_INW(port) audio_inw(port)
#define AUDIO_OUTW(value, port) audio_outw(value, port)

// Часто используемые аудиопорты
#define SB_DSP_RESET       0x06
#define SB_DSP_READ        0x0A
#define SB_DSP_WRITE       0x0C
#define SB_DSP_STATUS      0x0E
#define SB_MIXER_REG       0x04
#define SB_MIXER_DATA      0x05

#endif // PORT_IO_AUDIO_H