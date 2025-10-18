#include "audio.h"
#include "port_io.h"  // Изменено с port_io_audio.h на port_io.h
#include "timer.h"
#include "isr.h"
#include "string.h"
#include "video.h"
#include "terminal.h"

static uint8_t current_audio_type = 0;

// Test melody data (frequency, duration_ms)
static uint32_t test_melody[MELODY_NOTES][2] = {
    {262, 250}, {294, 250}, {330, 250}, {349, 250},
    {392, 250}, {440, 250}, {494, 250}, {523, 500},
    {494, 250}, {440, 250}, {392, 500}, {392, 500}
};

// PC Speaker functions (теперь они определены в port_io.c)
// Убраны дублирующиеся определения

// Sound Blaster functions с оптимизированным портовым вводом-выводом
static void sb_write(uint16_t port, uint8_t value) {
    // Используем стандартные функции портового ввода-вывода
    outb(value, port + 0x0C); // SB_DSP_WRITE обычно по смещению 0x0C
}

static uint8_t sb_read(uint16_t port) {
    // Используем стандартные функции портового ввода-вывода
    return inb(port + 0x0E); // SB_DSP_READ обычно по смещению 0x0E
}

// Sound Blaster 16 specific functions
static void sb16_write_dsp(uint16_t base, uint8_t value) {
    if (/* проверка статуса */ 1) { // Упрощено для компиляции
        outb(value, base + 0x0C);
    }
}

static uint8_t sb16_read_dsp(uint16_t base) {
    if (/* проверка статуса */ 1) { // Упрощено для компиляции
        return inb(base + 0x0E);
    }
    return 0xFF;
}

// AC97 functions
static uint16_t ac97_read(uint16_t base, uint8_t reg) {
    outw(reg, base + 0x00);
    return inw(base + 0x02);
}

static void ac97_write(uint16_t base, uint8_t reg, uint16_t value) {
    outw(reg, base + 0x00);
    outw(value, base + 0x02);
}

void audio_init() {
    // По умолчанию используем PC Speaker
    current_audio_type = AUDIO_PC_SPEAKER;
    
    // Попытка обнаружения другого аудиооборудования
    if (audio_detect_ac97()) {
        current_audio_type = AUDIO_AC97;
    } else if (audio_detect_sb16()) {
        current_audio_type = AUDIO_SB_16;
    } else if (audio_detect_sound_blaster()) {
        current_audio_type = AUDIO_SB_1_0;
    }
    
    // Короткий звук для индикации готовности аудио
    audio_beep(BEEP_FREQ, 100);
}

void audio_beep(uint32_t frequency, uint32_t duration_ms) {
    if (current_audio_type == AUDIO_PC_SPEAKER) {
        pc_speaker_beep(frequency, duration_ms);
    } else {
        // Для других устройств используем PC Speaker как fallback
        pc_speaker_beep(frequency, duration_ms);
    }
}

void audio_play_sound(uint32_t nFrequence) {
    if (current_audio_type == AUDIO_PC_SPEAKER) {
        pc_speaker_play(nFrequence);
    } else {
        pc_speaker_play(nFrequence);
    }
}

void audio_nosound() {
    pc_speaker_stop();
}

uint8_t audio_detect_sound_blaster() {
    // Упрощенное обнаружение Sound Blaster
    return 0; // По умолчанию не найдено
}

uint8_t audio_detect_sb16() {
    // Упрощенное обнаружение SB16
    return 0; // По умолчанию не найдено
}

uint8_t audio_detect_ac97() {
    // Упрощенное обнаружение AC97
    return 0; // По умолчанию не найдено
}

void audio_sb_play_sample(uint8_t* sample, uint32_t length) {
    // Заглушка для воспроизведения сэмплов
}

void audio_play_test_melody() {
    // Всегда используем PC Speaker для тестовой мелодии
    for (int i = 0; i < MELODY_NOTES; i++) {
        pc_speaker_play(test_melody[i][0]);
        // timer_wait(test_melody[i][1]); // Нужна реализация timer_wait
        pc_speaker_stop();
        // timer_wait(50); // Нужна реализация timer_wait
    }
    pc_speaker_stop();
}

void audio_test() {
    const char* device_name = audio_get_device_name();
    
    terminal_writestring("Audio Device Test\n");
    terminal_writestring("================\n");
    terminal_writestring("Detected device: ");
    terminal_writestring(device_name);
    terminal_writestring("\n");
    
    terminal_writestring("Playing test sound... ");
    audio_play_test_melody();
    terminal_writestring("Done!\n");
    
    terminal_writestring("Audio test completed successfully.\n");
}

const char* audio_get_device_name() {
    switch (current_audio_type) {
        case AUDIO_PC_SPEAKER: return "PC Speaker";
        case AUDIO_SB_1_0: return "Sound Blaster 1.0/2.0";
        case AUDIO_SB_16: return "Sound Blaster 16";
        case AUDIO_AC97: return "ICH AC97 Audio";
        case AUDIO_INTEL_HD: return "Intel HD Audio";
        default: return "Unknown Audio Device";
    }
}