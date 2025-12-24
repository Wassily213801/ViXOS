#include "string.h"
#include <stdint.h>
#include <stddef.h>

size_t strlen(const char* str) {
    int len = 0;
    while (str[len] != 0) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, int n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    return n ? *(unsigned char*)s1 - *(unsigned char*)s2 : 0;
}

char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while (*src) {
        *dest++ = *src++;
    }
    *dest = 0;
    return ret;
}

void* memset(void* dest, int value, unsigned int len) {
    unsigned char* ptr = (unsigned char*)dest;
    while (len-- > 0) {
        *ptr++ = (unsigned char)value;
    }
    return dest;
}

char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == (char)c)
            return (char*)str;
        str++;
    }
    return 0;
}

char* strtok(char* str, const char* delim) {
    static char* last;
    if (str) last = str;
    if (!last) return 0;

    char* token_start = last;
    while (*token_start && strchr(delim, *token_start)) token_start++;

    if (!*token_start) {
        last = 0;
        return 0;
    }

    char* token_end = token_start;
    while (*token_end && !strchr(delim, *token_end)) token_end++;

    if (*token_end) {
        *token_end = 0;
        last = token_end + 1;
    } else {
        last = 0;
    }

    return token_start;
}

void* memcpy(void* dest, const void* src, uint32_t count) {
    uint8_t* dst8 = (uint8_t*)dest;
    const uint8_t* src8 = (const uint8_t*)src;
    for (uint32_t i = 0; i < count; i++) {
        dst8[i] = src8[i];
    }
    return dest;
}

const char* strrchr(const char* s, int c) {
    const char* last = NULL;
    if (s == NULL) return NULL;
    while (*s) {
        if (*s == (char)c) {
            last = s;
        }
        s++;
    }
    return last;
}

// Реализация strncpy
char* strncpy(char* dest, const char* src, size_t n) {
    char* ret = dest;
    while (n-- && (*dest++ = *src++));
    while (n-- > 0) *dest++ = '\0';
    return ret;
}

// Реализация strstr
char* strstr(const char* haystack, const char* needle) {
    if (!haystack || !needle || !*needle) {
        return (char*)haystack;
    }
    
    const char* haystack_ptr;
    const char* needle_ptr;
    
    for (; *haystack; haystack++) {
        haystack_ptr = haystack;
        needle_ptr = needle;
        
        while (*haystack_ptr && *needle_ptr && (*haystack_ptr == *needle_ptr)) {
            haystack_ptr++;
            needle_ptr++;
        }
        
        if (!*needle_ptr) {
            return (char*)haystack;
        }
    }
    
    return NULL;
}

void itoa(int value, char* str, int base) {
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    int tmp_value;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    int is_negative = 0;
    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }

    while (value != 0) {
        tmp_value = value % base;
        *ptr++ = (tmp_value < 10) ? (tmp_value + '0') : (tmp_value - 10 + 'A');
        value /= base;
    }

    if (is_negative)
        *ptr++ = '-';

    *ptr-- = '\0';

    // Реверс строки
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

// 64-битное беззнаковое деление
uint64_t __udivdi3(uint64_t num, uint64_t den) {
    uint64_t quot = 0;
    uint64_t rem = 0;
    
    for (int i = 63; i >= 0; i--) {
        rem = (rem << 1) | ((num >> i) & 1);
        if (rem >= den) {
            rem -= den;
            quot |= (1ULL << i);
        }
    }
    
    return quot;
}

// 64-битный беззнаковый остаток
uint64_t __umoddi3(uint64_t num, uint64_t den) {
    uint64_t rem = 0;
    
    for (int i = 63; i >= 0; i--) {
        rem = (rem << 1) | ((num >> i) & 1);
        if (rem >= den) {
            rem -= den;
        }
    }
    
    return rem;
}

// Реализация strcat
char* strcat(char* dest, const char* src) {
    char* ptr = dest;
    
    // Находим конец строки dest
    while (*ptr != '\0') {
        ptr++;
    }
    
    // Копируем src в конец dest
    while (*src != '\0') {
        *ptr++ = *src++;
    }
    
    // Добавляем завершающий нуль
    *ptr = '\0';
    
    return dest;
}

// Реализация atoi
int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    // Пропускаем пробелы
    while (*str == ' ') str++;
    
    // Проверяем знак
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Конвертируем цифры
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}
