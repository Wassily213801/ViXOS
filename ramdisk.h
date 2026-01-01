#ifndef RAMDISK_H
#define RAMDISK_H

#include <stdint.h>

#define MAX_FILES 128  // Увеличили для системных файлов
#define MAX_FILENAME 64
#define MAX_FILE_CONTENT 1024  // Увеличили для логов и конфигураций

typedef struct {
    char name[MAX_FILENAME];
    char owner[16];
    char created[16];
    uint32_t size;
    char content[MAX_FILE_CONTENT];
    int is_directory;
    int is_readonly;    // Файл только для чтения
    int is_system;      // Системный файл (нельзя удалить)
} RamdiskEntry;

void ramdisk_init();
int ramdisk_add_file(const char* path, const char* content);
int ramdisk_delete_file(const char* path);
int ramdisk_write_file(const char* path, const char* content);
void ramdisk_ls();
const char* ramdisk_read_file(const char* path);
void ramdisk_list_files();
int ramdisk_save(const char* filename);
int ramdisk_load(const char* filename);
void ramdisk_show_meta(const char* path);
int ramdisk_mkdir(const char* name); 
int ramdisk_is_dir(const char* path);
int ramdisk_exists(const char* path);
int ramdisk_is_readonly(const char* path);
int ramdisk_is_system(const char* path);
void ramdisk_protect_file(const char* path);
void ramdisk_unprotect_file(const char* path);
void ramdisk_list_system_files();

#endif