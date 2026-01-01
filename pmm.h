#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include "multiboot.h"  // Добавляем этот заголовок

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
void pmm_init_from_multiboot(multiboot_info_t* mbi);  // Исправлено!
void pmm_init_with_memory_map(uint32_t total_memory_kb);
void* pmm_alloc_block();
void* pmm_alloc_blocks(uint32_t count);
void pmm_free_block(void* block);
void pmm_free_blocks(void* block, uint32_t count);

// Функции для получения информации о памяти
uint32_t pmm_get_total_memory();
uint32_t pmm_get_used_memory();
uint32_t pmm_get_free_memory();
memory_info_t pmm_get_memory_info();
const char* pmm_get_memory_type();

// Объявление функции обнаружения памяти через BIOS
uint32_t detect_memory(void);  // Добавлено!

#endif
