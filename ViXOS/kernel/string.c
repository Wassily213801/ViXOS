#include "string.h"

void* memset(void* dest, int val, size_t count) {
    unsigned char* ptr = dest;
    while (count--) {
        *ptr++ = (unsigned char)val;
    }
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}