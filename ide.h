#ifndef IDE_H
#define IDE_H

#include <stdint.h>
#include <stddef.h>

#define IDE_PRIMARY_BASE 0x1F0
#define IDE_SECONDARY_BASE 0x170

// IDE Registers
#define IDE_REG_DATA        0x00
#define IDE_REG_ERROR       0x01
#define IDE_REG_FEATURES    0x01
#define IDE_REG_SECTOR_COUNT 0x02
#define IDE_REG_LBA_LOW     0x03
#define IDE_REG_LBA_MID     0x04
#define IDE_REG_LBA_HIGH    0x05
#define IDE_REG_DEVICE      0x06
#define IDE_REG_COMMAND     0x07
#define IDE_REG_STATUS      0x07
#define IDE_REG_ALT_STATUS  0x206

// IDE Commands
#define IDE_CMD_READ_SECTORS  0x20
#define IDE_CMD_WRITE_SECTORS 0x30
#define IDE_CMD_IDENTIFY      0xEC

// Status Register Bits
#define IDE_STATUS_ERR   (1 << 0)
#define IDE_STATUS_DRQ   (1 << 3)
#define IDE_STATUS_SRV   (1 << 4)
#define IDE_STATUS_DF    (1 << 5)
#define IDE_STATUS_RDY   (1 << 6)
#define IDE_STATUS_BSY   (1 << 7)

typedef struct {
    uint8_t present;
    uint8_t primary;
    uint8_t slave;
    uint32_t total_sectors;
    uint16_t cylinders;
    uint16_t heads;
    uint16_t sectors_per_track;
    char model[41];
} ide_drive_t;

extern ide_drive_t ide_disk_info;

// Function prototypes
void ide_init();
uint8_t ide_detect_drive(uint16_t base, uint8_t slave);
void ide_identify_drive(uint16_t base, uint8_t slave);
uint8_t ide_wait_ready(uint16_t base);
uint8_t ide_wait_drq(uint16_t base);
void ide_read_sectors(uint32_t lba, uint8_t num_sectors, uint16_t* buffer);
void ide_write_sectors(uint32_t lba, uint8_t num_sectors, uint16_t* buffer);
uint8_t ide_check_disk_presence();

#endif