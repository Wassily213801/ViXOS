#include "ide.h"
#include "port_io.h"
#include "string.h"
#include "video.h"

ide_drive_t ide_disk_info = {0};

static uint16_t ide_current_base = IDE_PRIMARY_BASE;

// Wait for IDE controller to be ready
uint8_t ide_wait_ready(uint16_t base) {
    uint8_t status;
    int timeout = 100000; // Timeout counter
    
    while (timeout-- > 0) {
        status = inb(base + IDE_REG_STATUS);
        if (!(status & IDE_STATUS_BSY)) {
            return 1; // Ready
        }
    }
    return 0; // Timeout
}

// Wait for data request
uint8_t ide_wait_drq(uint16_t base) {
    uint8_t status;
    int timeout = 100000;
    
    while (timeout-- > 0) {
        status = inb(base + IDE_REG_STATUS);
        if (status & IDE_STATUS_DRQ) {
            return 1; // Data ready
        }
        if (status & IDE_STATUS_ERR) {
            return 0; // Error
        }
    }
    return 0; // Timeout
}

// Detect if drive exists
uint8_t ide_detect_drive(uint16_t base, uint8_t slave) {
    // Select drive
    outb(base + IDE_REG_DEVICE, 0xA0 | (slave << 4));
    
    // Wait a bit
    for (int i = 0; i < 4; i++) {
        inb(base + IDE_REG_ALT_STATUS);
    }
    
    // Read status
    uint8_t status1 = inb(base + IDE_REG_STATUS);
    uint8_t status2 = inb(base + IDE_REG_STATUS);
    
    // Check if status makes sense
    if (status1 == 0xFF || status2 == 0xFF) {
        return 0; // No controller
    }
    
    // Select drive again
    outb(base + IDE_REG_DEVICE, 0xA0 | (slave << 4));
    
    if (!ide_wait_ready(base)) {
        return 0;
    }
    
    return 1;
}

// Identify drive
void ide_identify_drive(uint16_t base, uint8_t slave) {
    if (!ide_detect_drive(base, slave)) {
        return;
    }
    
    // Select drive
    outb(base + IDE_REG_DEVICE, 0xA0 | (slave << 4));
    
    if (!ide_wait_ready(base)) {
        return;
    }
    
    // Send IDENTIFY command
    outb(base + IDE_REG_COMMAND, IDE_CMD_IDENTIFY);
    
    if (!ide_wait_ready(base)) {
        return;
    }
    
    uint8_t status = inb(base + IDE_REG_STATUS);
    if (status == 0) {
        return; // No drive
    }
    
    // Wait for data
    if (!ide_wait_drq(base)) {
        return;
    }
    
    // Read identify data
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(base + IDE_REG_DATA);
    }
    
    // Extract model name
    for (int i = 0; i < 20; i++) {
        ide_disk_info.model[i * 2] = (identify_data[27 + i] >> 8) & 0xFF;
        ide_disk_info.model[i * 2 + 1] = identify_data[27 + i] & 0xFF;
    }
    ide_disk_info.model[40] = '\0';
    
    // Trim spaces from model name
    for (int i = 39; i >= 0; i--) {
        if (ide_disk_info.model[i] == ' ') {
            ide_disk_info.model[i] = '\0';
        } else {
            break;
        }
    }
    
    // Get drive geometry
    ide_disk_info.cylinders = identify_data[1];
    ide_disk_info.heads = identify_data[3];
    ide_disk_info.sectors_per_track = identify_data[6];
    
    // Calculate total sectors (LBA28)
    if (identify_data[83] & (1 << 10)) {
        // LBA48 supported, but we'll use LBA28 for simplicity
        ide_disk_info.total_sectors = 
            (identify_data[61] << 16) | identify_data[60];
    } else {
        // CHS to LBA28 conversion
        ide_disk_info.total_sectors = 
            ide_disk_info.cylinders * 
            ide_disk_info.heads * 
            ide_disk_info.sectors_per_track;
    }
    
    ide_disk_info.present = 1;
    ide_disk_info.primary = (base == IDE_PRIMARY_BASE) ? 1 : 0;
    ide_disk_info.slave = slave;
    ide_current_base = base;
}

void ide_init() {
    
    // Try primary master
    if (ide_detect_drive(IDE_PRIMARY_BASE, 0)) {
        ide_identify_drive(IDE_PRIMARY_BASE, 0);
        return;
    }
    
    // Try primary slave
    if (ide_detect_drive(IDE_PRIMARY_BASE, 1)) {
        ide_identify_drive(IDE_PRIMARY_BASE, 1);
        return;
    }
    
    // Try secondary master
    if (ide_detect_drive(IDE_SECONDARY_BASE, 0)) {
        ide_identify_drive(IDE_SECONDARY_BASE, 0);
        return;
    }
    
    // Try secondary slave
    if (ide_detect_drive(IDE_SECONDARY_BASE, 1)) {

        ide_identify_drive(IDE_SECONDARY_BASE, 1);
        return;
    }

}

// Check if disk is present
uint8_t ide_check_disk_presence() {
    return ide_disk_info.present;
}

// Read sectors from disk
void ide_read_sectors(uint32_t lba, uint8_t num_sectors, uint16_t* buffer) {
    if (!ide_disk_info.present) {
        return;
    }
    
    if (!ide_wait_ready(ide_current_base)) {
        return;
    }
    
    // Set sector count
    outb(ide_current_base + IDE_REG_SECTOR_COUNT, num_sectors);
    
    // Set LBA address
    outb(ide_current_base + IDE_REG_LBA_LOW, (lba >> 0) & 0xFF);
    outb(ide_current_base + IDE_REG_LBA_MID, (lba >> 8) & 0xFF);
    outb(ide_current_base + IDE_REG_LBA_HIGH, (lba >> 16) & 0xFF);
    
    // Set device and LBA bits
    outb(ide_current_base + IDE_REG_DEVICE, 0xE0 | ((lba >> 24) & 0x0F) | (ide_disk_info.slave << 4));
    
    // Send read command
    outb(ide_current_base + IDE_REG_COMMAND, IDE_CMD_READ_SECTORS);
    
    // Read sectors
    for (int sector = 0; sector < num_sectors; sector++) {
        if (!ide_wait_drq(ide_current_base)) {
            return;
        }
        
        // Read 256 words (512 bytes)
        for (int i = 0; i < 256; i++) {
            buffer[sector * 256 + i] = inw(ide_current_base + IDE_REG_DATA);
        }
    }
}

// Write sectors to disk (basic implementation)
void ide_write_sectors(uint32_t lba, uint8_t num_sectors, uint16_t* buffer) {
    if (!ide_disk_info.present) {
        return;
    }
    
    if (!ide_wait_ready(ide_current_base)) {
        return;
    }
    
    // Set sector count
    outb(ide_current_base + IDE_REG_SECTOR_COUNT, num_sectors);
    
    // Set LBA address
    outb(ide_current_base + IDE_REG_LBA_LOW, (lba >> 0) & 0xFF);
    outb(ide_current_base + IDE_REG_LBA_MID, (lba >> 8) & 0xFF);
    outb(ide_current_base + IDE_REG_LBA_HIGH, (lba >> 16) & 0xFF);
    
    // Set device and LBA bits
    outb(ide_current_base + IDE_REG_DEVICE, 0xE0 | ((lba >> 24) & 0x0F) | (ide_disk_info.slave << 4));
    
    // Send write command
    outb(ide_current_base + IDE_REG_COMMAND, IDE_CMD_WRITE_SECTORS);
    
    // Write sectors
    for (int sector = 0; sector < num_sectors; sector++) {
        if (!ide_wait_drq(ide_current_base)) {
            return;
        }
        
        // Write 256 words (512 bytes)
        for (int i = 0; i < 256; i++) {
            outw(ide_current_base + IDE_REG_DATA, buffer[sector * 256 + i]);
        }
        
        // Wait for write to complete
        if (!ide_wait_ready(ide_current_base)) {
            return;
        }
    }
}