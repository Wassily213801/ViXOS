#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PMM_BLOCK_SIZE 4096
#define PMM_MAX_BLOCKS 131072  // Увеличиваем для поддержки большего объема памяти

// Структура для информации о памяти
typedef struct {
    uint32_t total_memory;
    uint32_t available_memory;
    uint32_t used_memory;
    uint32_t total_blocks;
    uint32_t used_blocks;
} memory_info_t;

void pmm_init();
void pmm_init_with_memory_map(uint32_t total_memory_kb);
void* pmm_alloc_block();
void pmm_free_block(void* block);

// Функции для получения информации о памяти
uint32_t pmm_get_total_memory();
uint32_t pmm_get_used_memory();
uint32_t pmm_get_free_memory();
memory_info_t pmm_get_memory_info();
const char* pmm_get_memory_type();

#endif