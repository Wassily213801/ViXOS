#ifndef TIME_H
#define TIME_H

#include <stdint.h>

// Структура для хранения времени и даты
typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t weekday;
} datetime_t;

// Функции для работы с портами (добавлено)
static inline void outb(uint16_t port, uint8_t value);
static inline uint8_t inb(uint16_t port);

// Функции для работы со временем
void time_init();
datetime_t get_vienna_time();
void time_command();
const char* get_weekday_name(uint8_t weekday);
const char* get_month_name(uint8_t month);

#endif