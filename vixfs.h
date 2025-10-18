#ifndef VIXFS_H
#define VIXFS_H

#include <stdint.h>
#include <stddef.h>

#define VIXFS_MAGIC 0x56495846  // "VIXF"
#define VIXFS_VERSION 1
#define VIXFS_MAX_FILENAME 128
#define VIXFS_MAX_FILES 256
#define VIXFS_BLOCK_SIZE 512

// Структура суперблока
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t free_blocks;
    uint32_t inode_count;
    uint32_t free_inodes;
    uint32_t root_inode;
    uint8_t padding[480]; // Дополнение до 512 байт
} vixfs_superblock_t;

// Структура индексного дескриптора
typedef struct {
    uint32_t mode;
    uint32_t size;
    uint32_t blocks;
    uint32_t block_pointers[12];
    uint32_t indirect_block;
    uint32_t double_indirect_block;
    uint64_t created_time;
    uint64_t modified_time;
    uint8_t name[VIXFS_MAX_FILENAME];
} vixfs_inode_t;

// Структура директории
typedef struct {
    uint32_t inode;
    uint8_t name[VIXFS_MAX_FILENAME];
} vixfs_direntry_t;

// Основные функции файловой системы
void vixfs_init(void);
int vixfs_format(void);
int vixfs_mount(void);
int vixfs_create(const char* filename);
int vixfs_delete(const char* filename);
int vixfs_read(const char* filename, void* buffer, size_t size);
int vixfs_write(const char* filename, const void* data, size_t size);
int vixfs_list_files(void);

// Вспомогательные функции
uint32_t vixfs_alloc_block(void);
void vixfs_free_block(uint32_t block);
uint32_t vixfs_alloc_inode(void);
void vixfs_free_inode(uint32_t inode);
vixfs_inode_t* vixfs_find_file(const char* filename);

#endif