#include "terminal.h"
#include "video.h"
#include "string.h"
#include "ramdisk.h"
#include "keyboard.h"
#include "sys.h"
#include "kernel_panic.h"
#include "login.h"
#include "gfx_window.h"
#include "gui.h" 
#include "gfx.h"
#include "ide.h"
#include "audio.h"
#include "serial.h"
#include "guess.h"
#include "calculator.h"
#include "pmm.h"
#include "time.h"
#include "vixfs.h"
#include "snake.h"
#include "shutdown_screen.h"
#include "ahci.h"
#include "license.h"  
void gui_command();
void calculator_command();
void update_prompt();
void read_file_content(char* buffer, int max_len);
void handle_command(const char* cmd_line);
void autocomplete();
void terminal_init();
void terminal_run();
void terminal_writestring(const char* str);
void terminal_puts(const char* str);
void terminal_putchar(char c);
void terminal_writehex(uint32_t value);
void print_memory_info(const char* label, uint64_t bytes);
// Объявление функции gui_command() которая определена в gui.c
extern void gui_command(void);
#define MAX_HISTORY 10
#define MAX_COMMAND_LENGTH 128
#define MAX_DIRS 16
#define MAX_NAME_LEN 32
#define MAX_PATH_LENGTH 128
#define GUESS_MIN_NUMBER 1
#define GUESS_MAX_NUMBER 100
#define GUESS_MAX_ATTEMPTS 10

typedef struct {
    char name[MAX_NAME_LEN];
} Directory;

static Directory dirs[MAX_DIRS];
static int dir_count = 1;
static char cwd[MAX_PATH_LENGTH] = "vix";
static char history[MAX_HISTORY][MAX_COMMAND_LENGTH];
static int history_count = 0;
static int history_index = 0;
static char command[MAX_COMMAND_LENGTH];
static int command_len = 0;
extern int gui_mode;

const char* commands[] = {
    "help", "clear", "version", "off", "reboot",
    "ls", "mkdir", "echo", "cat", "read",
    "add", "rm", "save", "load", "meta",
    "cd", "logoff", "audio", "panic", "guess",
    "calc", "meminfo", "time", "ide",
    "vixfs", "vixcreate", "vixdelete", "vixlist", "vixwrite", "vixread", "snake", //"ahci"
};
const int num_commands = 33;

// Добавляем вспомогательную функцию для форматирования вывода памяти
void print_memory_info(const char* label, uint64_t bytes) {
    char buffer[32];
    
    if (bytes < 1024) {
        itoa((uint32_t)bytes, buffer, 10);
        video_print(label);
        video_print(buffer);
        video_print(" bytes\n");
    } else if (bytes < 1024 * 1024) {
        uint32_t kb = bytes / 1024;
        itoa(kb, buffer, 10);
        video_print(label);
        video_print(buffer);
        video_print(" KB\n");
    } else if (bytes < 1024 * 1024 * 1024) {
        uint32_t mb = bytes / (1024 * 1024);
        itoa(mb, buffer, 10);
        video_print(label);
        video_print(buffer);
        video_print(" MB\n");
    } else {
        uint32_t gb = bytes / (1024 * 1024 * 1024);
        uint32_t mb_remainder = (bytes % (1024 * 1024 * 1024)) / (1024 * 1024);
        itoa(gb, buffer, 10);
        video_print(label);
        video_print(buffer);
        video_print(".");
        
        // Выводим один знак после запятой для GB
        itoa(mb_remainder / 100, buffer, 10);
        video_print(buffer);
        video_print(" GB\n");
    }
}
void ahci_command() {
    video_print("\n");
    
    // Инициализируем AHCI если еще не инициализирован
    if (!ahci_is_initialized()) {
        video_print("[AHCI] Initializing driver...\n");
        ahci_set_debug(1); // Включаем отладочный вывод
        ahci_init();
    }
    
    ahci_print_devices();
    
    // Обновляем промпт только если не в GUI режиме
    if (!gui_mode) {
        update_prompt();
    }
}
void ide_command() {
    video_print("\n");
    
    if (!ide_ctrl.initialized) {
        ide_init();
    }
    
    // Print in the requested format
    for (int channel = 0; channel < 2; channel++) {
        video_print("ide");
        video_putc('0' + channel);
        video_print(": ");
        
        // Master drive (hda)
        if (ide_ctrl.channels[channel].drives[1].present) {
            video_print("hda=");
            const char* type = ide_get_drive_type_name(
                ide_ctrl.channels[channel].drives[1].type);
            video_print(type);
        } else {
            video_print("hda=none");
        }
        
        video_print("  ");
        
        // Slave drive (hdb)
        if (ide_ctrl.channels[channel].drives[0].present) {
            video_print("hdb=");
            const char* type = ide_get_drive_type_name(
                ide_ctrl.channels[channel].drives[0].type);
            video_print(type);
        } else {
            video_print("hdb=none");
        }
        
        video_print("\n");
    }
    
    video_print("\n");
    update_prompt();
}
void update_prompt() {
    if (gui_mode) {
        wm_terminal_writestring(cwd);
        wm_terminal_writestring("> ");
    } else {
        video_print(cwd);
        video_print("> ");
    }
}
void read_file_content(char* buffer, int max_len) {
    int len = 0;
    video_print("Enter content: ");
    
    while (1) {
        char key = keyboard_getchar();
        
        if (key == '\n') {
            video_putc('\n');
            buffer[len] = '\0';
            break;
        } else if (key == '\b') {
            if (len > 0) {
                len--;
                video_backspace();
            }
        } else if (len < max_len - 1) {
            buffer[len++] = key;
            video_putc(key);
        }
    }
}

void handle_command(const char* cmd_line) {
    char buffer[MAX_COMMAND_LENGTH];
    strcpy(buffer, cmd_line);
    char* cmd = strtok(buffer, " ");
    char* arg1 = strtok(NULL, " ");
    char* arg2 = strtok(NULL, "");

    if (!cmd) return;

    if (strcmp(cmd, "help") == 0) {
        video_print("Available commands:\n");
        video_print("help, clear, version, off, reboot, ls, cd, mkdir\n");
        video_print("echo, cat <file>, read <file>, add <file>\n");
        video_print("rm <file>, save, load, meta <file> logoff, audio, guess, meminfo, time, snake,\n");
        video_print("panic <message>, calc, ideinfo,\n");
        video_print("vixfs, vixcreate <file>, vixdelete <file>, vixlist, vixwrite <file> <data>, vixread <file>\n");
    } else if (strcmp(cmd, "add") == 0) {
        if (!arg1) {
            video_print("Usage: add <filename>\n");
        } else {
            char content[MAX_FILE_CONTENT];
            read_file_content(content, sizeof(content));
            
            if (ramdisk_add_file(arg1, content) == 0) {
                video_set_color(0x0A, 0x00); // Зеленый
                video_print("File added successfully.\n");
                video_set_color(0x07, 0x00); // Белый
            } else {
                video_set_color(0x0C, 0x00); // Красный
                video_print("Failed to add file.\n");
                video_set_color(0x07, 0x00); // Белый
            }
        }
    } else if (strcmp(cmd, "read") == 0 || strcmp(cmd, "cat") == 0) {
        if (!arg1) {
            video_print("Usage: read <file>\n");
        } else {
            const char* content = ramdisk_read_file(arg1);
            if (content) {
                video_print(content);
                video_print("\n");
            } else {
                video_print("File not found or is a directory\n");
            }
        }
    } else if (strcmp(cmd, "rm") == 0) {
        if (!arg1) {
            video_print("Usage: rm <file>\n");
        } else {
            if (ramdisk_delete_file(arg1) == 0) {
                video_set_color(0x0A, 0x00); // Зеленый
                video_print("File deleted successfully.\n");
                video_set_color(0x07, 0x00); // Белый
            } else {
                video_set_color(0x0C, 0x00); // Красный
                video_print("Failed to delete file.\n");
                video_set_color(0x07, 0x00); // Белый
            }
        }
    } else if (strcmp(cmd, "save") == 0) {
        ramdisk_save("ramdisk.bin");
    } else if (strcmp(cmd, "version") == 0) {
        video_print("ViX Kernel 0.3 Beta\n");
        video_print("Copyright (C) 2025-2026 ViXOS Developers\n");
        video_print("Version: 26.7 Alpha Beta\n");
        video_print("Build Date: 2026-01-01\n");
        video_print("Architecture: i386\n");
        video_print("code name: Nova\n");
        video_print("License: GPLv2\n");
    } else if (strcmp(cmd, "load") == 0) {
        ramdisk_load("ramdisk.bin");
    } else if (strcmp(cmd, "off") == 0 || strcmp(cmd, "shutdown") == 0) {
    // Просто вызываем shutdown_screen, который сам вызовет shutdown()
    // и покажет соответствующее сообщение
    shutdown_screen();
}else if (strcmp(cmd, "reboot") == 0) {
    video_clear();
    reboot_screen();

    reboot();
    } else if (strcmp(cmd, "mkdir") == 0) {
        if (!arg1) {
            video_print("Usage: mkdir <name>\n");
        } else {
            if (ramdisk_mkdir(arg1) == 0) {
                video_set_color(0x0A, 0x00); // Зеленый
                video_print("Folder created\n");
                video_set_color(0x07, 0x00); // Белый
            } else {
                video_set_color(0x0C, 0x00); // Красный
                video_print("Failed to create folder\n");
                video_set_color(0x07, 0x00); // Белый
            }
        }
    } else if (strcmp(cmd, "meta") == 0) {
        if (!arg1) {
            video_print("Usage: meta <file>\n");
        } else {
            ramdisk_show_meta(arg1);
        }
    } else if (strcmp(cmd, "clear") == 0) {
        video_clear();
        terminal_init();
    } else if (strcmp(cmd, "ls") == 0) {
        ramdisk_ls();
    } else if (strcmp(cmd, "cd") == 0) {
        if (!arg1) {
            strcpy(cwd, "vix");
        } else {
            if (ramdisk_is_dir(arg1)) {
                strcpy(cwd, arg1);
            } else if (strcmp(arg1, "..") == 0) {
                strcpy(cwd, "vix");
            } else {
                video_print("Directory not found: ");
                video_print(arg1);
                video_print("\n");
            }
        }
    } else if (strcmp(cmd, "panic") == 0) {
        panic_command(arg1); 
    } else if (strcmp(cmd, "logoff") == 0) {
        video_clear();
        display_welcome_menu();
        handle_welcome_menu();
        terminal_init();
        terminal_run();
    } else if (strcmp(cmd, "guess") == 0) {
        guess_game();
    } else if (strcmp(cmd, "calc") == 0) {
        calculator_command();
    } else if (strcmp(cmd, "meminfo") == 0) {
        video_print("Memory Information:\n");
        video_print("==================\n");
        
        memory_info_t mem_info = pmm_get_memory_info();
        const char* mem_type = pmm_get_memory_type();
        
        // Общая информация о памяти
        video_print("Memory Type:    ");
        video_print(mem_type);
        video_print("\n");
        
        // Детальная информация в разных единицах измерения
        video_print("\nDetailed Memory Usage:\n");
        video_print("----------------------\n");
        
        print_memory_info("Total memory:  ", mem_info.total_memory);
        print_memory_info("Used memory:   ", mem_info.used_memory);
        print_memory_info("Free memory:   ", mem_info.available_memory);
        
        // Процент использования
        uint32_t usage_percent = 0;
        if (mem_info.total_memory > 0) {
            usage_percent = (mem_info.used_memory * 100) / mem_info.total_memory;
        }
        
        char percent_str[16];
        itoa(usage_percent, percent_str, 10);
        video_print("Usage:          ");
        video_print(percent_str);
        video_print("%\n");
        
        // Информация о блоках
        video_print("\nMemory Blocks:\n");
        video_print("--------------\n");
        video_print("Block size:    4096 bytes\n");
        
        char blocks_info[64];
        itoa(mem_info.total_blocks, blocks_info, 10);
        video_print("Total blocks:  ");
        video_print(blocks_info);
        video_print("\n");
        
        itoa(mem_info.used_blocks, blocks_info, 10);
        video_print("Used blocks:   ");
        video_print(blocks_info);
        video_print("\n");
        
        itoa(mem_info.total_blocks - mem_info.used_blocks, blocks_info, 10);
        video_print("Free blocks:   ");
        video_print(blocks_info);
        video_print("\n");
        
        video_print("\nPhysical Memory:\n");
        video_print("----------------\n");
        
        uint32_t total_kb = mem_info.total_memory / 1024;
        char mem_kb_str[32];
        itoa(total_kb, mem_kb_str, 10);
        video_print("Detected RAM:  ");
        video_print(mem_kb_str);
        video_print(" KB\n");
        
        uint32_t total_mb = total_kb / 1024;
        itoa(total_mb, mem_kb_str, 10);
        video_print("              ");
        video_print(mem_kb_str);
        video_print(" MB\n");
        
        if (total_mb >= 1024) {
            uint32_t total_gb = total_mb / 1024;
            uint32_t remainder_mb = total_mb % 1024;
            itoa(total_gb, mem_kb_str, 10);
            video_print("              ");
            video_print(mem_kb_str);
            video_print(".");
            itoa(remainder_mb / 10, mem_kb_str, 10); // Один знак после запятой
            video_print(mem_kb_str);
            video_print(" GB\n");
        }
    } else if (strcmp(cmd, "time") == 0) {
        time_command();
    }else if (strcmp(cmd, "vixfs") == 0) {
    vixfs_init();
} else if (strcmp(cmd, "vixcreate") == 0) {
    if (!arg1) {
        video_print("Usage: vixcreate <filename>\n");
    } else {
        vixfs_create(arg1);
    }
} else if (strcmp(cmd, "vixdelete") == 0) {
    if (!arg1) {
        video_print("Usage: vixdelete <filename>\n");
    } else {
        vixfs_delete(arg1);
    }
} else if (strcmp(cmd, "vixlist") == 0) {
    vixfs_list_files();
} else if (strcmp(cmd, "vixwrite") == 0) {
    if (!arg1 || !arg2) {
        video_print("Usage: vixwrite <filename> <data>\n");
    } else {
        vixfs_write(arg1, arg2, strlen(arg2));
    }
} else if (strcmp(cmd, "vixread") == 0) {
    if (!arg1) {
        video_print("Usage: vixread <filename>\n");
    } else {
        char buffer[1024];
        int bytes_read = vixfs_read(arg1, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            video_print("File content: ");
            video_print(buffer);
            video_print("\n");
        }
    }
} else if (strcmp(cmd, "snake") == 0) {
    snake_game();
} else if (strcmp(cmd, "gui" )== 0) {
    video_print("[TERM] handling 'gui' command\n");
    serial_write("[TERM] handling 'gui' command\n");
    gui_command();
    video_print("[TERM] returned from gui_command()\n");
    serial_write("[TERM] returned from gui_command()\n");
}
    //else if (strcmp(cmd, "ahci") == 0) {
        //ahci_command();
    //}// В handle_command добавьте улучшенную версию:
//else if (strcmp(cmd, "ide") == 0) {
    //video_print("\n");
    
    //if (!ide_is_initialized()) {
        //video_print("IDE driver not initialized. Initializing...\n");
       // ide_init();
   // }
    
    //ide_print_devices();
    
    // Показываем информацию об ошибках, если есть
    //uint32_t error = ide_get_last_error();
    //if (error != IDE_ERROR_NONE) {
       // video_print("Last error: ");
       // video_print(ide_get_error_string(error));
       // video_print("\n");
   // }
//} 
// Добавьте этот блок в функцию handle_command(), например после "snake":
else if (strcmp(cmd, "gpl") == 0 || strcmp(cmd, "license") == 0) {
    show_gpl_license();
}else if (strcmp(cmd, "sysfiles") == 0) {
        ramdisk_list_system_files();
    } else if (strcmp(cmd, "protect") == 0) {
        if (!arg1) {
            video_print("Usage: protect <filename>\n");
        } else {
            ramdisk_protect_file(arg1);
            video_print("File protected as read-only\n");
        }
    } else if (strcmp(cmd, "unprotect") == 0) {
        if (!arg1) {
            video_print("Usage: unprotect <filename>\n");
        } else {
            ramdisk_unprotect_file(arg1);
            video_print("File write permission restored\n");
        }
    } else if (strcmp(cmd, "write") == 0) {
        if (!arg1) {
            video_print("Usage: write <filename> [content]\n");
        } else if (!arg2) {
            // Интерактивный ввод
            char content[MAX_FILE_CONTENT];
            read_file_content(content, sizeof(content));
            
            if (ramdisk_write_file(arg1, content) == 0) {
                video_set_color(0x0A, 0x00);
                video_print("File written successfully.\n");
                video_set_color(0x07, 0x00);
            } else {
                video_set_color(0x0C, 0x00);
                video_print("Failed to write file.\n");
                video_set_color(0x07, 0x00);
            }
        } else {
            // Запись из аргумента команды
            if (ramdisk_write_file(arg1, arg2) == 0) {
                video_set_color(0x0A, 0x00);
                video_print("File written successfully.\n");
                video_set_color(0x07, 0x00);
            } else {
                video_set_color(0x0C, 0x00);
                video_print("Failed to write file.\n");
                video_set_color(0x07, 0x00);
            }
        }
    }else {
        // ЦВЕТНОЕ СООБЩЕНИЕ ДЛЯ НЕИЗВЕСТНОЙ КОМАНДЫ
        video_set_color(0x0C, 0x00); // Красный текст на черном фоне
        video_print("Unknown command: ");
        video_set_color(0x0E, 0x00); // Желтый текст для имени команды
        video_print(cmd);
        video_set_color(0x0C, 0x00); // Снова красный
        video_print(" - type 'help' for available commands\n");
        video_set_color(0x07, 0x00); // Возврат к белому тексту
    }
}

void autocomplete() {
    int matches = 0;
    const char* match = 0;
    command[command_len] = '\0';
    for (int i = 0; i < num_commands; ++i) {
        if (strncmp(commands[i], command, command_len) == 0) {
            if (!match) match = commands[i];
            matches++;
        }
    }

    if (matches == 1 && match) {
        while (command_len > 0) {
            video_backspace();
            command_len--;
        }
        strcpy(command, match);
        command_len = strlen(command);
        video_print(command);
    } else if (matches > 1) {
        video_putc('\n');
        for (int i = 0; i < num_commands; ++i) {
            if (strncmp(commands[i], command, command_len) == 0) {
                video_print(commands[i]);
                video_putc('\n');
            }
        }
        video_print(cwd);
        video_print("> ");
        video_print(command);
    }
}

void terminal_init() {
    if (gui_mode) {
        wm_create_terminal_window();
        wm_terminal_writestring("ViXOS Terminal\n");
        wm_terminal_writestring("Code Name Nova\n");
        wm_terminal_writestring("Build 37.29\n");
        wm_terminal_writestring("===========\n");
        wm_terminal_writestring("Development\n");
    } else {
        video_clear();
        video_print("ViXOS Terminal\n");
        video_print("Code Name Nova\n");
        video_print("Build 37.29\n");
        video_print("===========\n");
        video_print("Development\n");
        ramdisk_init();
    }
}

void terminal_run() {
    if (gui_mode) {
        update_prompt();
        while (1) {
            char key = keyboard_getchar();

            if (key == '\n') {
                wm_terminal_putchar('\n');
                command[command_len] = '\0';

                if (command_len > 0) {
                    if (history_count < MAX_HISTORY) {
                        strcpy(history[history_count++], command);
                    } else {
                        for (int i = 1; i < MAX_HISTORY; ++i)
                            strcpy(history[i - 1], history[i]);
                        strcpy(history[MAX_HISTORY - 1], command);
                    }
                }

                handle_command(command);
                command_len = 0;
                history_index = history_count;
                update_prompt();
            }
            else if (key == '\b') {
                if (command_len > 0) {
                    command_len--;
                    wm_terminal_putchar('\b');
                }
            }
            else if (key == '\t') {
                autocomplete();
            }
            else if (command_len < MAX_COMMAND_LENGTH - 1) {
                command[command_len++] = key;
                wm_terminal_putchar(key);
            }
        }
    } else {
        video_print(cwd);
        video_print("> ");
        
        while (1) {
            char key = keyboard_getchar();

            if (key == '\n') {
                video_putc('\n');
                command[command_len] = '\0';

                if (command_len > 0) {
                    if (history_count < MAX_HISTORY) {
                        strcpy(history[history_count++], command);
                    } else {
                        for (int i = 1; i < MAX_HISTORY; ++i)
                            strcpy(history[i - 1], history[i]);
                        strcpy(history[MAX_HISTORY - 1], command);
                    }
                }

                handle_command(command);
                command_len = 0;
                history_index = history_count;
                video_print(cwd);
                video_print("> ");
            }
            else if (key == '\b') {
                if (command_len > 0) {
                    command_len--;
                    video_backspace();
                }
            }
            else if (key == '\t') {
                autocomplete();
            }
            else if (key == 0x48) { // Стрелка вверх
                if (history_index > 0) {
                    history_index--;
                    while (command_len > 0) {
                        video_backspace();
                        command_len--;
                    }
                    strcpy(command, history[history_index]);
                    command_len = strlen(command);
                    video_print(command);
                }
            }
            else if (key == 0x50) { // Стрелка вниз
                if (history_index < history_count - 1) {
                    history_index++;
                    while (command_len > 0) {
                        video_backspace();
                        command_len--;
                    }
                    strcpy(command, history[history_index]);
                    command_len = strlen(command);
                    video_print(command);
                } else if (history_index == history_count - 1) {
                    history_index++;
                    while (command_len > 0) {
                        video_backspace();
                        command_len--;
                    }
                }
            }
            else if (command_len < MAX_COMMAND_LENGTH - 1) {
                command[command_len++] = key;
                video_putc(key);
            }
        }
    }
}

void terminal_writestring(const char* str) {
    while (*str) {
        terminal_putchar(*str++);
    }
}

void terminal_puts(const char* str) {
    while (*str) {
        terminal_putchar(*str++);
    }
}

void terminal_putchar(char c) {
    if (gui_mode) {
        wm_terminal_putchar(c);
    } else {
        video_putc(c);  
    }
}

void terminal_writehex(uint32_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[11];
    
    buffer[0] = '0';
    buffer[1] = 'x';
    
    for (int i = 7; i >= 0; i--) {
        buffer[9 - i] = hex_chars[(value >> (i * 4)) & 0xF];
    }
    buffer[10] = '\0';
    
    terminal_writestring(buffer);
}