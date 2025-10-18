#include "memory.h"

#define MEMORY_POOL_SIZE 0x100000 // 1MB пул памяти
static uint8_t memory_pool[MEMORY_POOL_SIZE];
static size_t memory_used = 0;

void* simple_malloc(size_t size) {
    if (memory_used + size > MEMORY_POOL_SIZE) {
        return NULL;
    }
    void* ptr = &memory_pool[memory_used];
    memory_used += size;
    return ptr;
}

void simple_free(void* ptr) {
    (void)ptr;
}