#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TIMER_FREQUENCY 1000  // 1000 Гц
#define TIMER_TICKS_PER_MS 1  // Количество тиков за миллисекунду

void timer_install();
void timer_wait(int ticks);
uint64_t timer_get_ticks(void);
void timer_delay_ms(uint32_t ms);

#endif
