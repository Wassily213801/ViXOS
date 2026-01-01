#include "multiboot.h"

// Multiboot header должен быть в отдельной секции
__attribute__((section(".multiboot")))
multiboot_header_t mb_header = {
    .magic = MULTIBOOT_HEADER_MAGIC,
    .flags = MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO,
    .checksum = -(MULTIBOOT_HEADER_MAGIC + (MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO))
};

// Вспомогательные функции для работы с multiboot
uint32_t get_memory_size(multiboot_info_t* mbi) {
    if (mbi->flags & (1 << 0)) {
        return (mbi->mem_upper + mbi->mem_lower) * 1024;
    }
    return 0;
}