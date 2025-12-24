#include "ide.h"
#include "port_io.h"
#include "string.h"
#include "video.h"
#include "timer.h"

// Глобальный IDE контроллер
ide_controller_t ide_ctrl = {0};

// Вспомогательная функция для получения имени типа диска
const char* ide_get_drive_type_name(uint8_t type) {
    switch(type) {
        case IDE_DRIVE_NONE:  return "none";
        case IDE_DRIVE_HDD:   return "hdd";
        case IDE_DRIVE_CDROM: return "cdrom";
        case IDE_DRIVE_ATAPI: return "atapi";
        default:              return "unknown";
    }
}

// Получение строки ошибки
const char* ide_get_error_string(uint32_t error) {
    switch(error) {
        case IDE_ERROR_NONE:         return "No error";
        case IDE_ERROR_TIMEOUT:      return "Timeout";
        case IDE_ERROR_NOT_FOUND:    return "Device not found";
        case IDE_ERROR_BAD_STATUS:   return "Bad status";
        case IDE_ERROR_DRIVE_FAULT:  return "Drive fault";
        default:                     return "Unknown error";
    }
}

// Получение последней ошибки
uint32_t ide_get_last_error(void) {
    return ide_ctrl.last_error;
}

// Сброс последней ошибки
void ide_reset_last_error(void) {
    ide_ctrl.last_error = IDE_ERROR_NONE;
}

// Проверка инициализации драйвера
uint8_t ide_is_initialized(void) {
    return ide_ctrl.initialized;
}

// Простая задержка для стабилизации портов
void ide_delay(uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        inb(IDE_PRIMARY_BASE + IDE_REG_ALT_STATUS);
    }
}

// Тест порта IDE
uint8_t ide_test_port(uint16_t port) {
    // Пробуем прочитать статус несколько раз
    uint8_t status1 = inb(port);
    ide_delay(10);
    uint8_t status2 = inb(port);
    ide_delay(10);
    uint8_t status3 = inb(port);
    
    // Если все три чтения вернули 0xFF, порт не существует
    if (status1 == 0xFF && status2 == 0xFF && status3 == 0xFF) {
        return 0;
    }
    
    // Если все три чтения вернули 0x00, тоже странно
    if (status1 == 0x00 && status2 == 0x00 && status3 == 0x00) {
        return 0;
    }
    
    return 1; // Порт отвечает
}

// Ожидание готовности контроллера (не busy)
uint8_t ide_wait_ready(uint16_t base) {
    uint32_t timeout = IDE_TIMEOUT_READY;
    
    while (timeout--) {
        uint8_t status = inb(base + IDE_REG_STATUS);
        
        // Если нет busy и нет drive fault, значит готов
        if (!(status & IDE_STATUS_BSY) && !(status & IDE_STATUS_DF)) {
            return 1;
        }
        
        // Если есть ошибка
        if (status & IDE_STATUS_ERR) {
            ide_ctrl.last_error = IDE_ERROR_DRIVE_FAULT;
            return 0;
        }
    }
    
    ide_ctrl.last_error = IDE_ERROR_TIMEOUT;
    return 0; // Timeout
}

// Ожидание готовности данных
uint8_t ide_wait_drq(uint16_t base) {
    uint32_t timeout = IDE_TIMEOUT_DRQ;
    
    while (timeout--) {
        uint8_t status = inb(base + IDE_REG_STATUS);
        
        if (status & IDE_STATUS_DRQ) {
            return 1;
        }
        
        if (status & IDE_STATUS_ERR) {
            uint8_t error = inb(base + IDE_REG_ERROR);
            // Можно добавить логирование ошибки
            ide_ctrl.last_error = IDE_ERROR_DRIVE_FAULT;
            return 0;
        }
        
        if (status & IDE_STATUS_DF) {
            ide_ctrl.last_error = IDE_ERROR_DRIVE_FAULT;
            return 0;
        }
    }
    
    ide_ctrl.last_error = IDE_ERROR_TIMEOUT;
    return 0; // Timeout
}

// Сброс IDE контроллера
void ide_reset_channel(uint16_t base, uint16_t ctrl) {
    // Отправляем сигнал сброса
    outb(0x04, ctrl);
    ide_delay(IDE_DELAY_COUNT);
    
    // Снимаем сигнал сброса
    outb(0x00, ctrl);
    ide_delay(IDE_DELAY_COUNT * 2);
    
    // Отключаем прерывания
    outb(0x02, ctrl);
}

// Проверка канала IDE
uint8_t ide_probe_channel(uint16_t base) {
    if (!ide_test_port(base)) {
        return 0; // Порт не отвечает
    }
    
    // Пробуем несколько раз прочитать статус
    for (int attempt = 0; attempt < IDE_MAX_RETRIES; attempt++) {
        uint8_t status = inb(base + IDE_REG_STATUS);
        
        // Проверяем различные возможные состояния
        if (status != 0xFF && status != 0x00) {
            return 1; // Канал существует
        }
        
        ide_delay(IDE_DELAY_COUNT);
    }
    
    return 0; // Канал не найден
}

// Проверка наличия диска на канале
uint8_t ide_probe_drive(uint16_t base, uint8_t master) {
    uint8_t device_reg, status1, status2, status3;
    
    // Выбираем устройство
    if (master) {
        device_reg = IDE_DEVICE_MASTER;
    } else {
        device_reg = IDE_DEVICE_SLAVE;
    }
    
    outb(device_reg, base + IDE_REG_DEVICE);
    ide_delay(IDE_DELAY_COUNT);
    
    // Читаем статус несколько раз
    status1 = inb(base + IDE_REG_STATUS);
    ide_delay(10);
    status2 = inb(base + IDE_REG_STATUS);
    ide_delay(10);
    status3 = inb(base + IDE_REG_STATUS);
    
    // Проверяем, есть ли ответ
    if (status1 == 0xFF && status2 == 0xFF && status3 == 0xFF) {
        return 0; // Нет ответа
    }
    
    // Проверяем устройство регистр
    uint8_t device = inb(base + IDE_REG_DEVICE);
    if ((device & 0xF0) != device_reg) {
        return 0; // Устройство не ответило корректно
    }
    
    // Ждем готовности
    if (!ide_wait_ready(base)) {
        return 0;
    }
    
    // Проверяем статус
    uint8_t status = inb(base + IDE_REG_STATUS);
    if (status == 0x00 || status == 0xFF) {
        return 0; // Нет устройства
    }
    
    return 1; // Устройство найдено
}

// Идентификация ATA/IDE диска
uint8_t ide_identify_drive(ide_drive_t *drive, uint16_t base, uint8_t master) {
    uint8_t status;
    uint16_t identify_data[256];
    uint8_t retries = IDE_MAX_RETRIES;
    
    while (retries--) {
        // Выбираем устройство
        if (master) {
            outb(IDE_DEVICE_MASTER | IDE_DEVICE_LBA, base + IDE_REG_DEVICE);
        } else {
            outb(IDE_DEVICE_SLAVE | IDE_DEVICE_LBA, base + IDE_REG_DEVICE);
        }
        
        ide_delay(IDE_DELAY_COUNT);
        
        // Ждем готовности
        if (!ide_wait_ready(base)) {
            continue; // Пробуем еще раз
        }
        
        // Отправляем команду IDENTIFY
        outb(IDE_CMD_IDENTIFY, base + IDE_REG_COMMAND);
        ide_delay(IDE_DELAY_COUNT);
        
        // Ждем не busy
        if (!ide_wait_ready(base)) {
            continue;
        }
        
        // Проверяем наличие устройства
        status = inb(base + IDE_REG_STATUS);
        if (status == 0) {
            // Это ATAPI устройство?
            return 0; // Попробуем ATAPI ниже
        }
        
        // Ждем готовности данных
        if (!ide_wait_drq(base)) {
            continue;
        }
        
        // Читаем данные IDENTIFY
        for (int i = 0; i < 256; i++) {
            identify_data[i] = inw(base + IDE_REG_DATA);
        }
        
        // Извлекаем информацию о модели
        for (int i = 0; i < 20; i++) {
            drive->model[i * 2] = (identify_data[27 + i] >> 8) & 0xFF;
            drive->model[i * 2 + 1] = identify_data[27 + i] & 0xFF;
        }
        drive->model[40] = '\0';
        
        // Убираем пробелы в конце
        for (int i = 39; i >= 0; i--) {
            if (drive->model[i] == ' ') {
                drive->model[i] = '\0';
            } else if (drive->model[i] != '\0') {
                break;
            }
        }
        
        // Извлекаем серийный номер
        for (int i = 0; i < 10; i++) {
            drive->serial[i * 2] = (identify_data[10 + i] >> 8) & 0xFF;
            drive->serial[i * 2 + 1] = identify_data[10 + i] & 0xFF;
        }
        drive->serial[20] = '\0';
        
        // Извлекаем ревизию прошивки
        for (int i = 0; i < 4; i++) {
            drive->revision[i * 2] = (identify_data[23 + i] >> 8) & 0xFF;
            drive->revision[i * 2 + 1] = identify_data[23 + i] & 0xFF;
        }
        drive->revision[8] = '\0';
        
        // Получаем геометрию диска
        drive->cylinders = identify_data[1];
        drive->heads = identify_data[3];
        drive->sectors_per_track = identify_data[6];
        
        // Получаем возможности
        drive->capabilities = identify_data[49];
        
        // Проверяем поддержку LBA48
        drive->lba48_supported = (identify_data[83] & (1 << 10)) ? 1 : 0;
        
        // Проверяем поддержку DMA
        drive->dma_supported = (identify_data[49] & (1 << 8)) ? 1 : 0;
        
        // Получаем наборы команд
        drive->command_sets = (identify_data[83] << 16) | identify_data[82];
        
        // Получаем общее количество секторов
        drive->total_sectors = (identify_data[61] << 16) | identify_data[60];
        
        // Получаем общее количество секторов для LBA48
        if (drive->lba48_supported) {
            drive->total_sectors48 = 
                ((uint64_t)identify_data[103] << 48) |
                ((uint64_t)identify_data[102] << 32) |
                ((uint64_t)identify_data[101] << 16) |
                identify_data[100];
        } else {
            drive->total_sectors48 = drive->total_sectors;
        }
        
        // Устанавливаем тип диска
        drive->type = IDE_DRIVE_HDD;
        drive->atapi = 0;
        drive->present = 1;
        
        return 1; // Успех
    }
    
    return 0; // Не удалось идентифицировать
}

// Идентификация ATAPI устройства (CD/DVD)
uint8_t ide_identify_atapi(ide_drive_t *drive, uint16_t base, uint8_t master) {
    uint8_t status;
    uint16_t identify_data[256];
    
    // Выбираем устройство
    if (master) {
        outb(IDE_DEVICE_MASTER, base + IDE_REG_DEVICE);
    } else {
        outb(IDE_DEVICE_SLAVE, base + IDE_REG_DEVICE);
    }
    
    ide_delay(IDE_DELAY_COUNT);
    
    // Ждем готовности
    if (!ide_wait_ready(base)) {
        return 0;
    }
    
    // Отправляем команду ATAPI IDENTIFY
    outb(IDE_CMD_ATAPI_IDENTIFY, base + IDE_REG_COMMAND);
    ide_delay(IDE_DELAY_COUNT);
    
    // Ждем не busy
    if (!ide_wait_ready(base)) {
        return 0;
    }
    
    // Проверяем статус
    status = inb(base + IDE_REG_STATUS);
    if (status == 0) {
        return 0; // Нет устройства
    }
    
    // Ждем готовности данных
    if (!ide_wait_drq(base)) {
        return 0;
    }
    
    // Читаем данные IDENTIFY
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(base + IDE_REG_DATA);
    }
    
    // Определяем тип ATAPI устройства
    if ((identify_data[0] & 0xFF00) == 0x5A00) {
        strcpy(drive->model, "ATAPI CD-ROM Drive");
        drive->type = IDE_DRIVE_CDROM;
    } else if ((identify_data[0] & 0xFF00) == 0xEB00) {
        strcpy(drive->model, "ATAPI CD-ROM Drive");
        drive->type = IDE_DRIVE_CDROM;
    } else if ((identify_data[0] & 0xFF00) == 0x1400) {
        strcpy(drive->model, "ATAPI CD-ROM Drive");
        drive->type = IDE_DRIVE_CDROM;
    } else {
        strcpy(drive->model, "ATAPI Device");
        drive->type = IDE_DRIVE_ATAPI;
    }
    
    drive->atapi = 1;
    drive->present = 1;
    
    return 1;
}

// Обнаружение всех дисков на всех каналах
void ide_detect_all(void) {
    uint16_t bases[2] = {IDE_PRIMARY_BASE, IDE_SECONDARY_BASE};
    uint16_t ctrls[2] = {IDE_PRIMARY_CTRL, IDE_SECONDARY_CTRL};
    
    // Сбрасываем счетчик дисков
    ide_ctrl.num_drives = 0;
    
    // Проверяем оба канала
    for (int channel = 0; channel < 2; channel++) {
        uint16_t base = bases[channel];
        uint16_t ctrl = ctrls[channel];
        
        // Инициализируем структуру канала
        ide_ctrl.channels[channel].base = base;
        ide_ctrl.channels[channel].ctrl = ctrl;
        ide_ctrl.channels[channel].irq = (channel == 0) ? 14 : 15;
        
        // Очищаем информацию о дисках
        memset(&ide_ctrl.channels[channel].drives[0], 0, sizeof(ide_drive_t) * 2);
        
        // Проверяем существование канала
        if (!ide_probe_channel(base)) {
            continue; // Канал не существует, переходим к следующему
        }
        
        // Сбрасываем канал
        ide_reset_channel(base, ctrl);
        
        // Проверяем master и slave устройства
        for (int master = 0; master <= 1; master++) {
            ide_drive_t *drive = &ide_ctrl.channels[channel].drives[master];
            
            // Проверяем наличие устройства
            if (ide_probe_drive(base, master)) {
                drive->primary = (channel == 0) ? 1 : 0;
                drive->master = master;
                
                // Пытаемся идентифицировать как ATA устройство
                if (ide_identify_drive(drive, base, master)) {
                    ide_ctrl.num_drives++;
                    continue;
                }
                
                // Если ATA не удалось, пробуем ATAPI
                if (ide_identify_atapi(drive, base, master)) {
                    ide_ctrl.num_drives++;
                    continue;
                }
                
                // Если не удалось идентифицировать, но устройство есть
                drive->present = 1;
                drive->type = IDE_DRIVE_HDD;
                strcpy(drive->model, "Unknown IDE Device");
                ide_ctrl.num_drives++;
            }
        }
    }
    
    ide_ctrl.initialized = 1;
}

// Вывод информации об устройствах IDE
void ide_print_devices(void) {
    if (!ide_ctrl.initialized) {
        video_print("IDE controller not initialized\n");
        return;
    }
    
    if (ide_ctrl.num_drives == 0) {
        video_print("No IDE drives detected\n");
        return;
    }
    
    video_print("IDE Devices:\n");
    video_print("============\n");
    
    for (int channel = 0; channel < 2; channel++) {
        video_print("ide");
        video_putc('0' + channel);
        video_print(": ");
        
        // Master drive (hda)
        if (ide_ctrl.channels[channel].drives[1].present) {
            video_print("hda=");
            video_print(ide_get_drive_type_name(
                ide_ctrl.channels[channel].drives[1].type));
        } else {
            video_print("hda=none");
        }
        
        video_print("  ");
        
        // Slave drive (hdb)
        if (ide_ctrl.channels[channel].drives[0].present) {
            video_print("hdb=");
            video_print(ide_get_drive_type_name(
                ide_ctrl.channels[channel].drives[0].type));
        } else {
            video_print("hdb=none");
        }
        
        video_print("\n");
        
        // Детальная информация для каждого диска
        for (int master = 1; master >= 0; master--) {
            ide_drive_t *drive = &ide_ctrl.channels[channel].drives[master];
            if (drive->present) {
                video_print("  ");
                video_print(master ? "Master: " : "Slave:  ");
                video_print(drive->model);
                
                if (drive->type == IDE_DRIVE_HDD && drive->total_sectors > 0) {
                    char buffer[32];
                    uint64_t size_bytes = drive->total_sectors * 512;
                    uint32_t size_mb = size_bytes / (1024 * 1024);
                    
                    video_print(" (");
                    itoa(size_mb, buffer, 10);
                    video_print(buffer);
                    video_print(" MB)");
                    
                    if (drive->lba48_supported) {
                        video_print(" LBA48");
                    }
                }
                
                video_print("\n");
            }
        }
    }
    
    video_print("\nTotal drives detected: ");
    char buf[16];
    itoa(ide_ctrl.num_drives, buf, 10);
    video_print(buf);
    video_print("\n");
}

// Инициализация IDE контроллера
void ide_init(void) {
    video_print("IDE: Initializing controller...\n");
    
    // Сбрасываем ошибки
    ide_reset_last_error();
    
    // Проверяем основные порты
    uint8_t primary_exists = ide_test_port(IDE_PRIMARY_BASE);
    uint8_t secondary_exists = ide_test_port(IDE_SECONDARY_BASE);
    
    if (!primary_exists && !secondary_exists) {
        video_print("IDE: No IDE controllers found\n");
        return;
    }
    
    if (primary_exists) {
        video_print("IDE: Primary controller found\n");
    }
    
    if (secondary_exists) {
        video_print("IDE: Secondary controller found\n");
    }
    
    // Обнаруживаем все диски
    ide_detect_all();
    
    if (ide_ctrl.initialized && ide_ctrl.num_drives > 0) {
        video_print("IDE: Controller initialized successfully\n");
        video_print("IDE: Found ");
        char buf[16];
        itoa(ide_ctrl.num_drives, buf, 10);
        video_print(buf);
        video_print(" drive(s)\n");
    } else {
        video_print("IDE: No drives detected\n");
    }
}

// Проверка наличия диска
uint8_t ide_check_disk_presence(uint8_t channel, uint8_t drive) {
    if (channel > 1 || drive > 1) {
        return 0;
    }
    
    if (!ide_ctrl.initialized) {
        return 0;
    }
    
    return ide_ctrl.channels[channel].drives[drive].present;
}

// Чтение секторов с диска
uint8_t ide_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, 
                        uint8_t num_sectors, uint16_t *buffer) {
    if (channel > 1 || drive > 1) {
        return 0;
    }
    
    if (!ide_check_disk_presence(channel, drive)) {
        ide_ctrl.last_error = IDE_ERROR_NOT_FOUND;
        return 0;
    }
    
    ide_drive_t *drive_info = &ide_ctrl.channels[channel].drives[drive];
    uint16_t base = ide_ctrl.channels[channel].base;
    
    // Ждем готовности
    if (!ide_wait_ready(base)) {
        return 0;
    }
    
    // Устанавливаем количество секторов
    outb(num_sectors, base + IDE_REG_SECTOR_COUNT);
    
    // Устанавливаем LBA адрес
    outb((lba >> 0) & 0xFF, base + IDE_REG_LBA_LOW);
    outb((lba >> 8) & 0xFF, base + IDE_REG_LBA_MID);
    outb((lba >> 16) & 0xFF, base + IDE_REG_LBA_HIGH);
    
    // Выбираем устройство и режим LBA
    uint8_t device_reg = (drive ? IDE_DEVICE_MASTER : IDE_DEVICE_SLAVE) | 
                        IDE_DEVICE_LBA | ((lba >> 24) & 0x0F);
    outb(device_reg, base + IDE_REG_DEVICE);
    
    // Отправляем команду чтения
    uint8_t command = IDE_CMD_READ_SECTORS;
    if (drive_info->lba48_supported && (lba >> 28) > 0) {
        command = IDE_CMD_READ_SECTORS_EXT;
        // Для LBA48 нужна дополнительная настройка
    }
    
    outb(command, base + IDE_REG_COMMAND);
    
    // Читаем данные
    for (int sector = 0; sector < num_sectors; sector++) {
        if (!ide_wait_drq(base)) {
            return 0;
        }
        
        for (int i = 0; i < 256; i++) {
            buffer[sector * 256 + i] = inw(base + IDE_REG_DATA);
        }
    }
    
    return 1;
}

// Запись секторов на диск
uint8_t ide_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba,
                         uint8_t num_sectors, uint16_t *buffer) {
    if (channel > 1 || drive > 1) {
        return 0;
    }
    
    if (!ide_check_disk_presence(channel, drive)) {
        ide_ctrl.last_error = IDE_ERROR_NOT_FOUND;
        return 0;
    }
    
    ide_drive_t *drive_info = &ide_ctrl.channels[channel].drives[drive];
    uint16_t base = ide_ctrl.channels[channel].base;
    
    // Ждем готовности
    if (!ide_wait_ready(base)) {
        return 0;
    }
    
    // Устанавливаем количество секторов
    outb(num_sectors, base + IDE_REG_SECTOR_COUNT);
    
    // Устанавливаем LBA адрес
    outb((lba >> 0) & 0xFF, base + IDE_REG_LBA_LOW);
    outb((lba >> 8) & 0xFF, base + IDE_REG_LBA_MID);
    outb((lba >> 16) & 0xFF, base + IDE_REG_LBA_HIGH);
    
    // Выбираем устройство и режим LBA
    uint8_t device_reg = (drive ? IDE_DEVICE_MASTER : IDE_DEVICE_SLAVE) | 
                        IDE_DEVICE_LBA | ((lba >> 24) & 0x0F);
    outb(device_reg, base + IDE_REG_DEVICE);
    
    // Отправляем команду записи
    uint8_t command = IDE_CMD_WRITE_SECTORS;
    if (drive_info->lba48_supported && (lba >> 28) > 0) {
        command = IDE_CMD_WRITE_SECTORS_EXT;
        // Для LBA48 нужна дополнительная настройка
    }
    
    outb(command, base + IDE_REG_COMMAND);
    
    // Записываем данные
    for (int sector = 0; sector < num_sectors; sector++) {
        if (!ide_wait_drq(base)) {
            return 0;
        }
        
        for (int i = 0; i < 256; i++) {
            outw(buffer[sector * 256 + i], base + IDE_REG_DATA);
        }
        
        // Ждем завершения записи
        if (!ide_wait_ready(base)) {
            return 0;
        }
    }
    
    return 1;
}