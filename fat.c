#include "fat.h"
#include "terminal.h"
#include <stdint.h>

#define FAT16_IMAGE_BASE_ADDRESS 0x100000

static fat16_boot_sector_t* g_boot_sector;
static fat16_directory_entry_t* g_root_directory;

void fat_init() {
    g_boot_sector = (fat16_boot_sector_t*)FAT16_IMAGE_BASE_ADDRESS;

    if (g_boot_sector->file_system_type[0] != 'F' || g_boot_sector->file_system_type[1] != 'A') {
        terminal_writestring("Error: Not a FAT16 filesystem.\n");
        return;
    }

    uint32_t root_dir_start_sector = g_boot_sector->reserved_sectors + (g_boot_sector->fat_copies * g_boot_sector->sectors_per_fat);
    g_root_directory = (fat16_directory_entry_t*)(FAT16_IMAGE_BASE_ADDRESS + (root_dir_start_sector * g_boot_sector->bytes_per_sector));
    
    terminal_writestring("FAT16 Filesystem Initialized.\n");
}

void fat_list_root_directory() {
    if (!g_boot_sector) {
        terminal_writestring("FAT not initialized.\n");
        return;
    }

    terminal_writestring("Root Directory Listing:\n");
    for (uint16_t i = 0; i < g_boot_sector->root_dir_entries; i++) {
        fat16_directory_entry_t* entry = &g_root_directory[i];

        if (entry->filename[0] == 0x00 || entry->filename[0] == 0xE5 || entry->attributes == 0x0F) {
            continue;
        }

        char filename[13];
        int k = 0;
        for (int j = 0; j < 8; j++) {
            if (entry->filename[j] == ' ') break;
            filename[k++] = entry->filename[j];
        }
        if (entry->extension[0] != ' ') {
            filename[k++] = '.';
            for (int j = 0; j < 3; j++) {
                if (entry->extension[j] == ' ') break;
                filename[k++] = entry->extension[j];
            }
        }
        filename[k++] = ' ';
        filename[k] = '\0';
        terminal_writestring(filename); 

        if (entry->attributes & 0x10) {
            terminal_writestring("<DIR>\n");
        } else {
            terminal_writestring("FILE\n");
        }
    }
}