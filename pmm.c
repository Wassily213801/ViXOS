#include "pmm.h"
#include "string.h"
#include "ports.h"

static uint8_t memory_bitmap[PMM_MAX_BLOCKS / 8];
static uint32_t total_blocks = 0;
static uint32_t used_blocks = 0;
static uint32_t total_memory_kb = 0;

// Вспомогательные функции для работы с битовой картой
static void set_bit(uint32_t bit) {
    if (bit < total_blocks) {
        memory_bitmap[bit / 8] |= (1 << (bit % 8));
        used_blocks++;
    }
}

static void clear_bit(uint32_t bit) {
    if (bit < total_blocks) {
        memory_bitmap[bit / 8] &= ~(1 << (bit % 8));
        if (used_blocks > 0) used_blocks--;
    }
}

static uint8_t test_bit(uint32_t bit) {
    if (bit >= total_blocks) return 1;
    return memory_bitmap[bit / 8] & (1 << (bit % 8));
}

// Функция для получения объема памяти через BIOS (вызывается из bootloader)
uint32_t detect_memory(void) {
    // Упрощенное обнаружение памяти - в реальной системе это делается через BIOS
    // Для совместимости возвращаем фиксированное значение или определяем через CMOS
    uint32_t memory_kb = 0;
    
    // Попытка определить память через CMOS
    outb(0x70, 0x30);
    uint8_t low = inb(0x71);
    outb(0x70, 0x31);
    uint8_t high = inb(0x71);
    
    memory_kb = (high << 8) | low;
    
    // Если не удалось определить через CMOS, используем значение по умолчанию
    if (memory_kb < 1024 || memory_kb > 65535) {
        memory_kb = 65536; // 64MB по умолчанию
    }
    
    return memory_kb;
}

// Получение информации о памяти через Multiboot
static uint32_t detect_memory_from_multiboot(multiboot_info_t* mbi) {
    if (mbi->flags & (1 << 6)) { // Флаг наличия карты памяти
        uint32_t total_mem = 0;
        multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbi->mmap_addr;
        
        while ((uint32_t)mmap < mbi->mmap_addr + mbi->mmap_length) {
            if (mmap->type == 1) { // Доступная память
                total_mem += mmap->len_low;
            }
            mmap = (multiboot_memory_map_t*)((uint32_t)mmap + mmap->size + sizeof(uint32_t));
        }
        return total_mem / 1024; // Конвертируем в KB
    }
    
    // Fallback: используем значения из multiboot структуры
    if (mbi->flags & (1 << 0)) {
        return ((mbi->mem_upper + mbi->mem_lower) * 1024) / 1024;
    }
    
    return 65536; // 64MB по умолчанию
}

void pmm_init_from_multiboot(multiboot_info_t* mbi) {
    total_memory_kb = detect_memory_from_multiboot(mbi);
    pmm_init_with_memory_map(total_memory_kb);
}

void pmm_init(void) {
    // Автоматическое обнаружение памяти
    total_memory_kb = detect_memory();
    pmm_init_with_memory_map(total_memory_kb);
}

void pmm_init_with_memory_map(uint32_t total_memory_kb) {
    total_memory_kb = total_memory_kb;
    
    // Вычисляем общее количество блоков
    uint32_t total_memory_bytes = total_memory_kb * 1024;
    total_blocks = total_memory_bytes / PMM_BLOCK_SIZE;
    
    // Ограничиваем максимальным количеством блоков
    if (total_blocks > PMM_MAX_BLOCKS) {
        total_blocks = PMM_MAX_BLOCKS;
    }
    
    // Инициализируем bitmap
    memset(memory_bitmap, 0, sizeof(memory_bitmap));
    used_blocks = 0;
    
    // Помечаем первые 1MB как использованные (для kernel, BIOS и т.д.)
    uint32_t reserved_blocks = (1 * 1024 * 1024) / PMM_BLOCK_SIZE;
    for (uint32_t i = 0; i < reserved_blocks && i < total_blocks; i++) {
        set_bit(i);
    }
}

void* pmm_alloc_block(void) {
    for (uint32_t i = 0; i < total_blocks; i++) {
        if (!test_bit(i)) {
            set_bit(i);
            return (void*)(i * PMM_BLOCK_SIZE);
        }
    }
    return 0;
}

void* pmm_alloc_blocks(uint32_t count) {
    for (uint32_t i = 0; i <= total_blocks - count; i++) {
        uint32_t j;
        for (j = 0; j < count; j++) {
            if (test_bit(i + j)) break;
        }
        if (j == count) {
            for (j = 0; j < count; j++) {
                set_bit(i + j);
            }
            return (void*)(i * PMM_BLOCK_SIZE);
        }
    }
    return 0;
}

void pmm_free_block(void* block) {
    uint32_t index = (uint32_t)block / PMM_BLOCK_SIZE;
    if (index < total_blocks) {
        clear_bit(index);
    }
}

void pmm_free_blocks(void* block, uint32_t count) {
    uint32_t index = (uint32_t)block / PMM_BLOCK_SIZE;
    for (uint32_t i = 0; i < count && (index + i) < total_blocks; i++) {
        clear_bit(index + i);
    }
}

// Реализация функций для получения информации о памяти
uint32_t pmm_get_total_memory(void) {
    return total_blocks * PMM_BLOCK_SIZE;
}

uint32_t pmm_get_used_memory(void) {
    return used_blocks * PMM_BLOCK_SIZE;
}

uint32_t pmm_get_free_memory(void) {
    return (total_blocks - used_blocks) * PMM_BLOCK_SIZE;
}

memory_info_t pmm_get_memory_info(void) {
    memory_info_t info;
    info.total_memory = pmm_get_total_memory();
    info.used_memory = pmm_get_used_memory();
    info.available_memory = pmm_get_free_memory();
    info.total_blocks = total_blocks;
    info.used_blocks = used_blocks;
    return info;
}

const char* pmm_get_memory_type(void) {
    // Упрощенное определение типа памяти
    if (total_memory_kb >= 1024 * 1024) { // 1GB+
        return "DDR3/DDR4";
    } else if (total_memory_kb >= 512 * 1024) { // 512MB+
        return "DDR2/DDR3";
    } else {
        return "SDRAM/DDR";
    }
}