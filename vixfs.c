#include "vixfs.h"
#include "pmm.h"
#include "terminal.h"
#include "string.h"
#include "port_io.h"

static vixfs_superblock_t superblock;
static vixfs_inode_t inodes[VIXFS_MAX_FILES];
static uint8_t block_bitmap[4096]; // Битовая карта для 32768 блоков
static uint8_t inode_bitmap[32];   // Битовая карта для 256 инодов

// Инициализация файловой системы
void vixfs_init(void) {
    terminal_writestring("Initializing ViXFS...\n");
    
    // Инициализация битовых карт
    memset(block_bitmap, 0, sizeof(block_bitmap));
    memset(inode_bitmap, 0, sizeof(inode_bitmap));
    
    // Попытка монтирования существующей ФС
    if (vixfs_mount() != 0) {
        terminal_writestring("No existing ViXFS found, formatting...\n");
        vixfs_format();
    }
}

// Форматирование файловой системы
int vixfs_format(void) {
    terminal_writestring("Formatting ViXFS...\n");
    
    // Инициализация суперблока
    superblock.magic = VIXFS_MAGIC;
    superblock.version = VIXFS_VERSION;
    superblock.block_size = VIXFS_BLOCK_SIZE;
    superblock.total_blocks = 32768; // 16MB при размере блока 512 байт
    superblock.free_blocks = superblock.total_blocks - 1; // Минус суперблок
    superblock.inode_count = VIXFS_MAX_FILES;
    superblock.free_inodes = VIXFS_MAX_FILES - 1; // Минус корневой инод
    superblock.root_inode = 0;
    
    // Инициализация корневого инода
    inodes[0].mode = 0xFFFF; // Каталог
    inodes[0].size = 0;
    inodes[0].blocks = 0;
    inodes[0].created_time = 0;
    inodes[0].modified_time = 0;
    strcpy((char*)inodes[0].name, "/");
    
    // Помечаем корневой инод как использованный
    inode_bitmap[0] = 1;
    
    // Помечаем блок 0 как использованный (суперблок)
    block_bitmap[0] = 1;
    
    terminal_writestring("ViXFS formatted successfully!\n");
    return 0;
}

// Монтирование файловой системы
int vixfs_mount(void) {
    // Проверяем магическое число
    if (superblock.magic != VIXFS_MAGIC) {
        return -1; // ФС не найдена
    }
    
    terminal_writestring("ViXFS mounted successfully!\n");
    return 0;
}

// Создание файла
int vixfs_create(const char* filename) {
    if (strlen(filename) >= VIXFS_MAX_FILENAME) {
        terminal_writestring("Filename too long!\n");
        return -1;
    }
    
    // Проверяем, не существует ли уже файл с таким именем
    if (vixfs_find_file(filename) != NULL) {
        terminal_writestring("File already exists!\n");
        return -1;
    }
    
    // Ищем свободный инод
    uint32_t inode_num = vixfs_alloc_inode();
    if (inode_num == (uint32_t)-1) {
        terminal_writestring("No free inodes!\n");
        return -1;
    }
    
    // Инициализируем инод
    vixfs_inode_t* inode = &inodes[inode_num];
    inode->mode = 0xFFFE; // Файл
    inode->size = 0;
    inode->blocks = 0;
    inode->created_time = 0;
    inode->modified_time = 0;
    strcpy((char*)inode->name, filename);
    
    terminal_writestring("File created: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    
    return 0;
}

// Удаление файла
int vixfs_delete(const char* filename) {
    vixfs_inode_t* inode = vixfs_find_file(filename);
    if (inode == NULL) {
        terminal_writestring("File not found!\n");
        return -1;
    }
    
    // Освобождаем блоки файла
    for (uint32_t i = 0; i < inode->blocks; i++) {
        vixfs_free_block(inode->block_pointers[i]);
    }
    
    // Освобождаем инод
    uint32_t inode_num = inode - inodes;
    vixfs_free_inode(inode_num);
    
    terminal_writestring("File deleted: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    
    return 0;
}

// Чтение файла
int vixfs_read(const char* filename, void* buffer, size_t size) {
    vixfs_inode_t* inode = vixfs_find_file(filename);
    if (inode == NULL) {
        terminal_writestring("File not found!\n");
        return -1;
    }
    
    // Здесь должна быть логика чтения блоков с диска
    // Для демонстрации просто заполняем нулями
    memset(buffer, 0, size > inode->size ? inode->size : size);
    
    return size > inode->size ? inode->size : size;
}

// Запись в файл
int vixfs_write(const char* filename, const void* data, size_t size) {
    vixfs_inode_t* inode = vixfs_find_file(filename);
    if (inode == NULL) {
        terminal_writestring("File not found!\n");
        return -1;
    }
    
    // Вычисляем необходимое количество блоков
    uint32_t required_blocks = (size + VIXFS_BLOCK_SIZE - 1) / VIXFS_BLOCK_SIZE;
    
    if (required_blocks > 12) {
        terminal_writestring("File too large!\n");
        return -1;
    }
    
    // Выделяем блоки
    for (uint32_t i = 0; i < required_blocks; i++) {
        if (i >= inode->blocks) {
            uint32_t block = vixfs_alloc_block();
            if (block == (uint32_t)-1) {
                terminal_writestring("No free blocks!\n");
                return -1;
            }
            inode->block_pointers[i] = block;
            inode->blocks++;
        }
    }
    
    // Здесь должна быть логика записи блоков на диск
    inode->size = size;
    inode->modified_time = 0;
    
    terminal_writestring("Data written to: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    
    return size;
}

// Список файлов
int vixfs_list_files(void) {
    terminal_writestring("Files in ViXFS:\n");
    terminal_writestring("===============\n");
    
    for (uint32_t i = 0; i < VIXFS_MAX_FILES; i++) {
        if (inode_bitmap[i / 8] & (1 << (i % 8))) {
            vixfs_inode_t* inode = &inodes[i];
            terminal_writestring(inode->name);
            terminal_writestring(" [");
            
            if (inode->mode == 0xFFFF) {
                terminal_writestring("DIR");
            } else {
                terminal_writestring("FILE");
                char size_str[16];
                itoa(inode->size, size_str, 10);
                terminal_writestring(", ");
                terminal_writestring(size_str);
                terminal_writestring(" bytes");
            }
            
            terminal_writestring("]\n");
        }
    }
    
    return 0;
}

// Вспомогательные функции
uint32_t vixfs_alloc_block(void) {
    for (uint32_t i = 0; i < sizeof(block_bitmap) * 8; i++) {
        if (!(block_bitmap[i / 8] & (1 << (i % 8)))) {
            block_bitmap[i / 8] |= (1 << (i % 8));
            superblock.free_blocks--;
            return i;
        }
    }
    return (uint32_t)-1;
}

void vixfs_free_block(uint32_t block) {
    if (block < sizeof(block_bitmap) * 8) {
        block_bitmap[block / 8] &= ~(1 << (block % 8));
        superblock.free_blocks++;
    }
}

uint32_t vixfs_alloc_inode(void) {
    for (uint32_t i = 0; i < VIXFS_MAX_FILES; i++) {
        if (!(inode_bitmap[i / 8] & (1 << (i % 8)))) {
            inode_bitmap[i / 8] |= (1 << (i % 8));
            superblock.free_inodes--;
            return i;
        }
    }
    return (uint32_t)-1;
}

void vixfs_free_inode(uint32_t inode) {
    if (inode < VIXFS_MAX_FILES) {
        inode_bitmap[inode / 8] &= ~(1 << (inode % 8));
        superblock.free_inodes++;
        memset(&inodes[inode], 0, sizeof(vixfs_inode_t));
    }
}

vixfs_inode_t* vixfs_find_file(const char* filename) {
    for (uint32_t i = 0; i < VIXFS_MAX_FILES; i++) {
        if (inode_bitmap[i / 8] & (1 << (i % 8))) {
            if (strcmp((char*)inodes[i].name, filename) == 0) {
                return &inodes[i];
            }
        }
    }
    return NULL;
}