#ifndef IDE_H
#define IDE_H

#include <stdint.h>
#include <stddef.h>

// IDE Channel Bases
#define IDE_PRIMARY_BASE   0x1F0
#define IDE_PRIMARY_CTRL   0x3F6
#define IDE_SECONDARY_BASE 0x170
#define IDE_SECONDARY_CTRL 0x376

// IDE Register Offsets
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
#define IDE_CMD_READ_SECTORS       0x20
#define IDE_CMD_READ_SECTORS_EXT   0x24
#define IDE_CMD_WRITE_SECTORS      0x30
#define IDE_CMD_WRITE_SECTORS_EXT  0x34
#define IDE_CMD_IDENTIFY           0xEC
#define IDE_CMD_IDENTIFY_PACKET    0xA1
#define IDE_CMD_PACKET             0xA0
#define IDE_CMD_ATAPI_IDENTIFY     0xA1
#define IDE_CMD_FLUSH_CACHE        0xE7
#define IDE_CMD_FLUSH_CACHE_EXT    0xEA

// Status Register Bits
#define IDE_STATUS_ERR   (1 << 0)  // Error
#define IDE_STATUS_IDX   (1 << 1)  // Index
#define IDE_STATUS_CORR  (1 << 2)  // Corrected data
#define IDE_STATUS_DRQ   (1 << 3)  // Data request ready
#define IDE_STATUS_SRV   (1 << 4)  // Overlapped mode service request
#define IDE_STATUS_DF    (1 << 5)  // Drive fault
#define IDE_STATUS_RDY   (1 << 6)  // Drive ready
#define IDE_STATUS_BSY   (1 << 7)  // Busy

// Device/Head Register Bits
#define IDE_DEVICE_LBA    0x40      // Use LBA addressing
#define IDE_DEVICE_MASTER 0xA0      // Select master drive
#define IDE_DEVICE_SLAVE  0xB0      // Select slave drive

// Drive Types
#define IDE_DRIVE_NONE    0
#define IDE_DRIVE_HDD     1
#define IDE_DRIVE_CDROM   2
#define IDE_DRIVE_ATAPI   3

// Timeout values (увеличены для совместимости)
#define IDE_TIMEOUT_READY 10000000   // 10 миллионов циклов
#define IDE_TIMEOUT_DRQ   10000000
#define IDE_DELAY_COUNT   4000       // Количество циклов для задержки

// Error codes
#define IDE_ERROR_NONE         0
#define IDE_ERROR_TIMEOUT      1
#define IDE_ERROR_NOT_FOUND    2
#define IDE_ERROR_BAD_STATUS   3
#define IDE_ERROR_DRIVE_FAULT  4

// Максимальное количество попыток
#define IDE_MAX_RETRIES        3

// Структура для информации об IDE диске
typedef struct {
    uint8_t present;           // Диск присутствует
    uint8_t type;              // Тип: IDE_DRIVE_HDD, IDE_DRIVE_CDROM и т.д.
    uint8_t primary;           // Primary (1) или Secondary (0) канал
    uint8_t master;            // Master (1) или Slave (0)
    uint32_t total_sectors;    // Всего секторов (LBA28)
    uint64_t total_sectors48;  // Всего секторов (LBA48)
    uint16_t cylinders;
    uint16_t heads;
    uint16_t sectors_per_track;
    char model[41];            // Модель диска
    char serial[21];           // Серийный номер
    char revision[9];          // Ревизия прошивки
    uint16_t capabilities;     // Возможности диска
    uint32_t command_sets;     // Поддерживаемые команды
    uint8_t lba48_supported;   // Поддержка LBA48
    uint8_t dma_supported;     // Поддержка DMA
    uint8_t atapi;             // ATAPI устройство
} ide_drive_t;

// Структура IDE канала
typedef struct {
    uint16_t base;            // Базовый порт
    uint16_t ctrl;            // Контрольный порт
    uint8_t irq;              // IRQ линии
    ide_drive_t drives[2];    // Master (0) и Slave (1)
} ide_channel_t;

// Глобальная структура IDE контроллера
typedef struct {
    ide_channel_t channels[2];  // Primary (0) и Secondary (1) каналы
    uint8_t initialized;        // Флаг инициализации драйвера
    uint8_t num_drives;         // Количество обнаруженных дисков
    uint32_t last_error;        // Код последней ошибки
} ide_controller_t;

// Прототипы функций
void ide_init(void);
void ide_reset(void);
void ide_detect_all(void);
void ide_print_devices(void);
uint8_t ide_probe_channel(uint16_t base);
uint8_t ide_probe_drive(uint16_t base, uint8_t master);
uint8_t ide_identify_drive(ide_drive_t *drive, uint16_t base, uint8_t master);
uint8_t ide_wait_ready(uint16_t base);
uint8_t ide_wait_drq(uint16_t base);
void ide_delay(uint32_t count);
uint8_t ide_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, 
                         uint8_t num_sectors, uint16_t *buffer);
uint8_t ide_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba,
                         uint8_t num_sectors, uint16_t *buffer);
uint8_t ide_check_disk_presence(uint8_t channel, uint8_t drive);
const char* ide_get_drive_type_name(uint8_t type);
const char* ide_get_error_string(uint32_t error);
uint32_t ide_get_last_error(void);
void ide_reset_last_error(void);
uint8_t ide_is_initialized(void);
uint8_t ide_test_port(uint16_t port);

// Глобальный экземпляр контроллера
extern ide_controller_t ide_ctrl;

#endif