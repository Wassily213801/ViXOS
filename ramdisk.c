#include "ramdisk.h"
#include "video.h"
#include "string.h"
#include <stddef.h>

static RamdiskEntry entries[MAX_FILES];
static int file_count = 0;

static const char* current_date() {
    return "2026-01-01";
}

// Функция для проверки, является ли файл системным (неудаляемым)
static int is_system_file(const char* path) {
    const char* system_files[] = {
        "init.vix", "motd.txt", "config.sys", "ata_status.txt", 
        "time_status.txt", "vixfs_status.txt", "kernel.sys",
        "bootloader.bin", "system.map", "kernel_info.txt",
        "syslog.txt", "drivers.txt", "version.info", "license.txt",
        "readme.txt", "help.txt", "credits.txt", "changelog.txt",
        "build_info.txt", "hardware.txt", "modules.txt", "services.txt",
        "errors.log", "debug.log", "network.conf", "users.db",
        "security.policy", "backup.conf", "recovery.bat", "update.info"
    };
    
    for (int i = 0; i < sizeof(system_files)/sizeof(system_files[0]); i++) {
        if (strcmp(path, system_files[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Функция для проверки, является ли файл только для чтения
static int is_readonly_file(const char* path) {
    const char* readonly_files[] = {
        "kernel.sys", "bootloader.bin", "system.map", "license.txt",
        "version.info", "credits.txt", "changelog.txt", "build_info.txt"
    };
    
    for (int i = 0; i < sizeof(readonly_files)/sizeof(readonly_files[0]); i++) {
        if (strcmp(path, readonly_files[i]) == 0) {
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
    
    // Добавляем дополнительные системные файлы
    ramdisk_add_file("kernel.sys", "ViXOS Kernel - Core operating system");
    ramdisk_add_file("bootloader.bin", "Bootloader executable");
    ramdisk_add_file("system.map", "Kernel symbol table");
    ramdisk_add_file("kernel_info.txt", "Kernel version: 0.3 Beta\nArchitecture: i386\nBuild: 37.28");
    ramdisk_add_file("syslog.txt", "System log file - stores system events");
    ramdisk_add_file("drivers.txt", "Loaded drivers:\n- ATA Driver\n- VGA Driver\n- Keyboard Driver\n- RTC Driver");
    ramdisk_add_file("version.info", "ViXOS 0.3 Beta\nCode Name: Nova\nBuild Date: 2026-01-01");
    ramdisk_add_file("license.txt", "GNU GENERAL PUBLIC LICENSE Version 2");
    ramdisk_add_file("readme.txt", "ViXOS Operating System\nRead the documentation for usage instructions");
    ramdisk_add_file("help.txt", "Type 'help' in terminal for available commands");
    ramdisk_add_file("credits.txt", "ViXOS Developers Team\n2025-2026");
    ramdisk_add_file("changelog.txt", "Version 0.3 Beta:\n- Added AHCI support\n- Improved GUI\n- Fixed memory leaks");
    ramdisk_add_file("build_info.txt", "Compiler: GCC 13.2.0\nBuild options: -O2 -m32\nTarget: i386-pc");
    ramdisk_add_file("hardware.txt", "Detected hardware:\n- CPU: i386 compatible\n- RAM: 64MB detected\n- VGA: VESA compatible");
    ramdisk_add_file("modules.txt", "Loaded kernel modules:\n- pmm (Physical Memory Manager)\n- vmm (Virtual Memory Manager)\n- scheduler");
    ramdisk_add_file("services.txt", "Running services:\n- Terminal service\n- Filesystem service\n- Time service");
    ramdisk_add_file("errors.log", "Error log - empty");
    ramdisk_add_file("debug.log", "Debug information");
    ramdisk_add_file("network.conf", "Network configuration:\nDHCP: disabled\nIP: 192.168.1.100");
    ramdisk_add_file("users.db", "System users database");
    ramdisk_add_file("security.policy", "Security policy: Medium\nUser permissions enforced");
    ramdisk_add_file("backup.conf", "Backup configuration");
    ramdisk_add_file("recovery.bat", "System recovery script");
    ramdisk_add_file("update.info", "Last update: 2026-01-01\nStatus: Up to date");
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

    // Проверяем, не пытаемся ли перезаписать системный файл
    for (int i = 0; i < file_count; i++) {
        if (strcmp(entries[i].name, path) == 0 && is_system_file(path)) {
            video_print("Cannot overwrite system file: ");
            video_print(path);
            video_print("\n");
            return -1;
        }
    }

    RamdiskEntry* e = &entries[file_count++];
    strcpy(e->name, path);
    strcpy(e->owner, "vix");
    strcpy(e->created, current_date());
    e->size = strlen(content);
    e->is_directory = 0;
    strcpy(e->content, content);
    e->is_readonly = is_readonly_file(path);
    e->is_system = is_system_file(path);

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

int ramdisk_write_file(const char* path, const char* content) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(entries[i].name, path) == 0) {
            // Проверяем, можно ли записывать в файл
            if (entries[i].is_readonly) {
                video_print("File is read-only: ");
                video_print(path);
                video_print("\n");
                return -1;
            }
            
            if (strlen(content) >= MAX_FILE_CONTENT) {
                video_print("Content too long\n");
                return -1;
            }
            
            strcpy(entries[i].content, content);
            entries[i].size = strlen(content);
            return 0;
        }
    }
    
    // Если файл не найден, создаем новый
    return ramdisk_add_file(path, content);
}

void ramdisk_ls() {
    for (int i = 0; i < file_count; i++) {
        // Показываем метки для системных и только для чтения файлов
        if (entries[i].is_system) {
            video_print("[SYS] ");
        } else if (entries[i].is_readonly) {
            video_print("[RO]  ");
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
        
        // Показываем дополнительные метки
        if (entries[i].is_system && entries[i].is_readonly) {
            video_print(" [SYSTEM/READ-ONLY]");
        } else if (entries[i].is_system) {
            video_print(" [SYSTEM]");
        } else if (entries[i].is_readonly) {
            video_print(" [READ-ONLY]");
        }
        
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
            
            // Показываем метки
            if (entries[i].is_system) {
                video_print(" [SYSTEM FILE]");
            }
            if (entries[i].is_readonly) {
                video_print(" [READ-ONLY]");
            }
            if (entries[i].is_directory) {
                video_print(" [DIRECTORY]");
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
            
            video_print("Permissions: ");
            if (entries[i].is_system) {
                video_print("System protected");
            } else if (entries[i].is_readonly) {
                video_print("Read-only");
            } else {
                video_print("Read/Write");
            }
            video_print("\n");

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
    e->is_system = 0;
    e->is_readonly = 0;

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

int ramdisk_is_readonly(const char* path) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(entries[i].name, path) == 0) {
            return entries[i].is_readonly;
        }
    }
    return 0;
}

int ramdisk_is_system(const char* path) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(entries[i].name, path) == 0) {
            return entries[i].is_system;
        }
    }
    return 0;
}

void ramdisk_protect_file(const char* path) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(entries[i].name, path) == 0) {
            entries[i].is_readonly = 1;
            return;
        }
    }
}

void ramdisk_unprotect_file(const char* path) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(entries[i].name, path) == 0 && !entries[i].is_system) {
            entries[i].is_readonly = 0;
            return;
        }
    }
}

void ramdisk_list_system_files() {
    video_print("System files (protected):\n");
    video_print("=========================\n");
    
    int count = 0;
    for (int i = 0; i < file_count; i++) {
        if (entries[i].is_system) {
            count++;
            video_print(entries[i].is_readonly ? "[SYS/RO] " : "[SYS] ");
            video_print(entries[i].name);
            video_print(" (");
            video_print(entries[i].owner);
            video_print(") ");
            video_print(entries[i].created);
            video_print("\n");
        }
    }
    
    char count_str[16];
    itoa(count, count_str, 10);
    video_print("\nTotal system files: ");
    video_print(count_str);
    video_print("\n");
}