#include "ramdisk.h"
#include "video.h"
#include "string.h"
#include <stddef.h>

static RamdiskEntry entries[MAX_FILES];
static int file_count = 0;

static const char* current_date() {
    return "2025-10-15";
}

// Функция для проверки, является ли файл системным (неудаляемым)
static int is_system_file(const char* path) {
    const char* system_files[] = {
        "init.vix", "motd.txt", "config.sys", "ata_status.txt", 
        "time_status.txt", "vixfs_status.txt"
    };
    
    for (int i = 0; i < sizeof(system_files)/sizeof(system_files[0]); i++) {
        if (strcmp(path, system_files[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void ramdisk_init() {
    file_count = 0;

    // Автоматически добавляем системные файлы при инициализации
    ramdisk_add_file("init.vix", "RAMDISK initialization script");
    ramdisk_add_file("motd.txt", "Welcome to ViXOS!");
    ramdisk_add_file("config.sys", "System configuration file");
    ramdisk_add_file("ata_status.txt", "ATA Driver loaded successfully");
    ramdisk_add_file("time_status.txt", "RTC Time Module loaded");
    ramdisk_add_file("vixfs_status.txt", "ViXFS Filesystem initialized");
}

int ramdisk_add_file(const char* path, const char* content) {
    if (file_count >= MAX_FILES) {
        video_print("RAMDISK FULL\n");
        return -1;
    }

    if (strlen(path) >= MAX_FILENAME || strlen(content) >= MAX_FILE_CONTENT) {
        video_print("Name/content too long\n");
        return -1;
    }

    RamdiskEntry* e = &entries[file_count++];
    strcpy(e->name, path);
    strcpy(e->owner, "vix");
    strcpy(e->created, current_date());
    e->size = strlen(content);
    e->is_directory = 0;
    strcpy(e->content, content);

    return 0;
}

int ramdisk_delete_file(const char* path) {
    // Проверяем, является ли файл системным
    if (is_system_file(path)) {
        video_print("Cannot delete system file: ");
        video_print(path);
        video_print("\n");
        return -1;
    }
    
    for (int i = 0; i < file_count; i++) {
        if (strcmp(entries[i].name, path) == 0) {
            for (int j = i; j < file_count - 1; j++) {
                entries[j] = entries[j + 1];
            }
            file_count--;
            return 0;
        }
    }
    video_print("File not found\n");
    return -1;
}

void ramdisk_ls() {
    for (int i = 0; i < file_count; i++) {
        // Показываем метку [SYS] для системных файлов
        if (is_system_file(entries[i].name)) {
            video_print("[SYS] ");
        } else {
            video_print(entries[i].is_directory ? "[DIR] " : "[FILE] ");
        }
        
        video_print(entries[i].name);
        video_print(" | size: ");

        char size_str[16];
        itoa(entries[i].size, size_str, 10);
        video_print(size_str);

        video_print(" | created: ");
        video_print(entries[i].created);
        video_print("\n");
    }
}

const char* ramdisk_read_file(const char* path) {
    for (int i = 0; i < file_count; i++) {
        if (!entries[i].is_directory && strcmp(entries[i].name, path) == 0) {
            return entries[i].content;
        }
    }
    return NULL;
}

int ramdisk_save(const char* filename) {
    video_print("RAMDISK saved to ");
    video_print(filename);
    video_print("\n");
    return 0;
}

int ramdisk_load(const char* filename) {
    video_print("RAMDISK loaded from ");
    video_print(filename);
    video_print("\n");
    return 0;
}

void ramdisk_show_meta(const char* path) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(entries[i].name, path) == 0) {
            video_print("Name: ");
            video_print(entries[i].name);
            
            // Показываем метку системного файла
            if (is_system_file(entries[i].name)) {
                video_print(" [SYSTEM]");
            }
            
            video_print("\nOwner: ");
            video_print(entries[i].owner);
            video_print("\nCreated: ");
            video_print(entries[i].created);
            video_print("\nSize: ");

            char size_str[16];
            itoa(entries[i].size, size_str, 10);
            video_print(size_str);
            video_print(" bytes\n");

            return;
        }
    }
    video_print("File not found\n");
}

int ramdisk_mkdir(const char* name) {
    if (file_count >= MAX_FILES) {
        video_print("RAMDISK FULL\n");
        return -1;
    }

    if (strlen(name) >= MAX_FILENAME) {
        video_print("Name too long\n");
        return -1;
    }

    RamdiskEntry* e = &entries[file_count++];
    strcpy(e->name, name);
    strcpy(e->owner, "vix");
    strcpy(e->created, current_date());
    e->size = 0;
    e->is_directory = 1;
    e->content[0] = '\0';

    return 0;
}

int ramdisk_is_dir(const char* path) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(entries[i].name, path) == 0 && entries[i].is_directory) {
            return 1;
        }
    }
    return 0;
}

int ramdisk_exists(const char* path) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(entries[i].name, path) == 0) {
            return 1;
        }
    }
    return 0;
}