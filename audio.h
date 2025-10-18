#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

#define AUDIO_PC_SPEAKER   0x01
#define AUDIO_SB_1_0       0x02
#define AUDIO_SB_16        0x03
#define AUDIO_AC97         0x04
#define AUDIO_INTEL_HD     0x05

#define BEEP_FREQ      1000
#define LOGIN_FREQ     800
#define ERROR_FREQ     400

#define MELODY_NOTES 12
#define MELODY_DURATION 3000

void audio_init();
void audio_beep(uint32_t frequency, uint32_t duration_ms);
void audio_play_sound(uint32_t nFrequence);
void audio_nosound();
uint8_t audio_detect_sound_blaster();
uint8_t audio_detect_sb16();
uint8_t audio_detect_ac97();
void audio_sb_play_sample(uint8_t* sample, uint32_t length);
void audio_play_test_melody();
void audio_test();
const char* audio_get_device_name();
void terminal_writestring(const char* str);
#endif