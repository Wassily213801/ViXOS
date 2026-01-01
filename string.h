#ifndef STRING_H
#define STRING_H
#include <stdint.h>
#include <stddef.h>

size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, int n);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
int atoi(const char* str);
void* memset(void* dest, int value, unsigned int len);
char* strtok(char* str, const char* delim);
char* strchr(const char* str, int c);
void* memcpy(void* dest, const void* src, uint32_t count);
char* strstr(const char* haystack, const char* needle);  // Добавлено объявление
const char* strrchr(const char* s, int c);  // Добавлено объявление
char* strncpy(char* dest, const char* src, size_t n);  // Добавлено объявление

// Универсальная функция itoa для разных типов
void itoa(int value, char* str, int base);
void itoa64(uint64_t value, char* str, int base);
uint64_t __udivdi3(uint64_t num, uint64_t den);
uint64_t __umoddi3(uint64_t num, uint64_t den);
#endif