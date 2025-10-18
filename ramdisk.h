#ifndef RAMDISK_H
#define RAMDISK_H

#include <stdint.h>

#define MAX_FILES 64
#define MAX_FILENAME 64
#define MAX_FILE_CONTENT 512

typedef struct {
    char name[MAX_FILENAME];
    char owner[16];
    char created[16];
    uint32_t size;
    char content[MAX_FILE_CONTENT];
    int is_directory;
} RamdiskEntry;

void ramdisk_init();
int ramdisk_add_file(const char* path, const char* content);
int ramdisk_delete_file(const char* path);
void ramdisk_ls();
const char* ramdisk_read_file(const char* path);
void ramdisk_list_files();
int ramdisk_save(const char* filename);
int ramdisk_load(const char* filename);
void ramdisk_show_meta(const char* path);
int ramdisk_mkdir(const char* name); 
int ramdisk_is_dir(const char* path);  // Добавлено
int ramdisk_exists(const char* path);  // Добавлено
void ramdisk_list_files();
int ramdisk_save(const char* filename);
int ramdisk_load(const char* filename);
#endif