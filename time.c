#include "time.h"
#include "video.h"
#include "string.h"

// CMOS порты
#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

// Регистры RTC
#define RTC_SECOND   0x00
#define RTC_MINUTE   0x02
#define RTC_HOUR     0x04
#define RTC_DAY      0x07
#define RTC_MONTH    0x08
#define RTC_YEAR     0x09
#define RTC_STATUS_A 0x0A
#define RTC_STATUS_B 0x0B

// Реализации функций для работы с портами (добавлено)
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static int is_cmos_updating() {
    outb(CMOS_ADDRESS, RTC_STATUS_A);
    return inb(CMOS_DATA) & 0x80;
}

static uint8_t get_rtc_register(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

datetime_t get_vienna_time() {
    datetime_t time;
    
    // Ждем, пока CMOS не обновляется
    while (is_cmos_updating());
    
    // Читаем значения из RTC
    time.second = get_rtc_register(RTC_SECOND);
    time.minute = get_rtc_register(RTC_MINUTE);
    time.hour = get_rtc_register(RTC_HOUR);
    time.day = get_rtc_register(RTC_DAY);
    time.month = get_rtc_register(RTC_MONTH);
    time.year = get_rtc_register(RTC_YEAR);
    
    // Получаем день недели
    time.weekday = get_rtc_register(0x06);
    
    // Конвертируем из BCD в binary если необходимо
    uint8_t status_b = get_rtc_register(RTC_STATUS_B);
    
    if (!(status_b & 0x04)) {
        // BCD формат
        time.second = (time.second & 0x0F) + ((time.second / 16) * 10);
        time.minute = (time.minute & 0x0F) + ((time.minute / 16) * 10);
        time.hour = ((time.hour & 0x0F) + (((time.hour & 0x70) / 16) * 10)) | (time.hour & 0x80);
        time.day = (time.day & 0x0F) + ((time.day / 16) * 10);
        time.month = (time.month & 0x0F) + ((time.month / 16) * 10);
        time.year = (time.year & 0x0F) + ((time.year / 16) * 10);
    }
    
    // Корректируем год (предполагаем 21 век)
    if (time.year < 80) {
        time.year += 2000;
    } else {
        time.year += 1900;
    }
    
    // Корректируем час для Венского времени (CET/CEST)
    uint8_t original_hour = time.hour;
    time.hour = (time.hour + 1) % 24; // Венское время = UTC+1
    
    // Простая проверка на летнее время (CEST)
    if (time.month >= 4 && time.month <= 9) {
        time.hour = (time.hour + 1) % 24;
    } else if (time.month == 3 && time.day >= 25) {
        time.hour = (time.hour + 1) % 24;
    } else if (time.month == 10 && time.day <= 25) {
        time.hour = (time.hour + 1) % 24;
    }
    
    // Если при корректировке перешли на следующий день
    if (time.hour < original_hour) {
        time.day++;
        if (time.day > 31) {
            time.day = 1;
            time.month++;
            if (time.month > 12) {
                time.month = 1;
                time.year++;
            }
        }
        time.weekday = (time.weekday + 1) % 7;
    }
    
    return time;
}

const char* get_weekday_name(uint8_t weekday) {
    static const char* weekdays[] = {
        "Sunday", "Monday", "Tuesday", "Wednesday", 
        "Thursday", "Friday", "Saturday"
    };
    
    if (weekday < 7) {
        return weekdays[weekday];
    }
    return "Unknown";
}

const char* get_month_name(uint8_t month) {
    static const char* months[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    
    if (month >= 1 && month <= 12) {
        return months[month - 1];
    }
    return "Unknown";
}

void time_command() {
    datetime_t vienna_time = get_vienna_time();
    
    video_print("\nVienna Time (Austria):\n");
    video_print("=====================\n");
    
    // Форматируем время
    char time_str[32];
    char date_str[32];
    
    // Время в формате HH:MM
    itoa(vienna_time.hour, time_str, 10);
    if (vienna_time.hour < 10) {
        char temp[32];
        strcpy(temp, "0");
        strcat(temp, time_str);
        strcpy(time_str, temp);
    }
    strcat(time_str, ":");
    
    char minute_str[8];
    itoa(vienna_time.minute, minute_str, 10);
    if (vienna_time.minute < 10) {
        strcat(time_str, "0");
    }
    strcat(time_str, minute_str);
    
    // Дата в формате DD.MM.YYYY
    itoa(vienna_time.day, date_str, 10);
    if (vienna_time.day < 10) {
        char temp[32];
        strcpy(temp, "0");
        strcat(temp, date_str);
        strcpy(date_str, temp);
    }
    strcat(date_str, ".");
    
    char month_str[8];
    itoa(vienna_time.month, month_str, 10);
    if (vienna_time.month < 10) {
        strcat(date_str, "0");
    }
    strcat(date_str, month_str);
    strcat(date_str, ".");
    
    char year_str[8];
    itoa(vienna_time.year, year_str, 10);
    strcat(date_str, year_str);
    
    // Выводим информацию
    video_print("Time:      ");
    video_print(time_str);
    
    // Определяем летнее/зимнее время
    if (vienna_time.month >= 4 && vienna_time.month <= 9) {
        video_print(" CEST\n");
    } else {
        video_print(" CET\n");
    }
    
    video_print("Date:      ");
    video_print(date_str);
    video_print("\n");
    
    video_print("Day:       ");
    video_print(get_weekday_name(vienna_time.weekday));
    video_print("\n");
    
    video_print("Month:     ");
    video_print(get_month_name(vienna_time.month));
    video_print("\n");
    
    video_print("Year:      ");
    video_print(year_str);
    video_print("\n");
    
    video_print("Location:  Vienna, Austria\n");
    video_print("Timezone:  Central European Time\n");
}

void time_init() {
    // Инициализация времени
}