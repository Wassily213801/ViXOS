#include "port_io_audio.h"

// Встроенные функции для минимизации накладных расходов
static inline void io_delay(void) {
    // Короткая задержка для стабилизации портов ввода-вывода
    __asm__ volatile ("jmp 1f\n1: jmp 1f\n1:" : : : "memory");
}

uint8_t audio_inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile (
        "inb %w1, %b0"
        : "=a"(ret)
        : "Nd"(port)
        : "memory"
    );
    return ret;
}

void audio_outb(uint8_t value, uint16_t port) {
    __asm__ volatile (
        "outb %b0, %w1"
        : 
        : "a"(value), "Nd"(port)
        : "memory"
    );
}

uint16_t audio_inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile (
        "inw %w1, %w0"
        : "=a"(ret)
        : "Nd"(port)
        : "memory"
    );
    return ret;
}

void audio_outw(uint16_t value, uint16_t port) {
    __asm__ volatile (
        "outw %w0, %w1"
        : 
        : "a"(value), "Nd"(port)
        : "memory"
    );
}

uint32_t audio_inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile (
        "inl %w1, %0"
        : "=a"(ret)
        : "Nd"(port)
        : "memory"
    );
    return ret;
}

void audio_outl(uint32_t value, uint16_t port) {
    __asm__ volatile (
        "outl %0, %w1"
        : 
        : "a"(value), "Nd"(port)
        : "memory"
    );
}

void audio_delay(uint32_t microseconds) {
    // Простая задержка на основе инструкций
    for (volatile uint32_t i = 0; i < microseconds * 10; i++) {
        __asm__ volatile ("nop");
    }
}

uint8_t audio_wait_for_status(uint16_t port, uint8_t mask, uint8_t expected, uint32_t timeout_us) {
    uint32_t end_time = timeout_us;
    while (end_time-- > 0) {
        uint8_t status = audio_inb(port);
        if ((status & mask) == expected) {
            return 1; // Успех
        }
        audio_delay(1); // Задержка 1 мкс
    }
    return 0; // Таймаут
}

void audio_write_sequence(uint16_t base_port, const uint8_t* data, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        // Ждем готовности DSP к записи
        if (!audio_wait_for_status(base_port + SB_DSP_STATUS, 0x80, 0x00, 1000)) {
            break; // Таймаут
        }
        audio_outb(data[i], base_port + SB_DSP_WRITE);
    }
}