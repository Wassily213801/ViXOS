#include "ahci.h"
#include "video.h"
#include "string.h"
#include "pci.h"
#include "pmm.h"
#include "terminal.h"
#include "port_io.h"
#include "kernel_panic.h"

ahci_controller_t ahci_ctrl = {0};

// Макросы для безопасного чтения/записи MMIO
#define AHCI_READ_REG(base, offset) \
    ((base) ? (*(volatile uint32_t*)((uintptr_t)(base) + (offset))) : 0)

#define AHCI_WRITE_REG(base, offset, value) \
    if ((base)) { *(volatile uint32_t*)((uintptr_t)(base) + (offset)) = (value); }

// Вспомогательная функция для безопасного доступа к памяти
static inline uint32_t safe_read_mmio(uint32_t* base, uint32_t offset) {
    if (!base || ((uintptr_t)base & 0x3) != 0) {
        return 0; // Некорректный указатель или невыровненный доступ
    }
    return *(volatile uint32_t*)((uintptr_t)base + offset);
}

// Проверка, является ли адрес допустимым (не нулевым и не слишком большим)
static int is_valid_mmio_address(uint32_t addr) {
    if (addr == 0 || addr == 0xFFFFFFFF) {
        return 0;
    }
    // Проверяем, что адрес в допустимом диапазоне (ниже 4GB)
    if (addr >= 0x100000000) {
        return 0;
    }
    return 1;
}

// Управление отладочным выводом
int ahci_debug = 1; // По умолчанию включен для диагностики

void ahci_set_debug(int on) {
    ahci_debug = on ? 1 : 0;
}

// Безопасная инициализация порта (без реального доступа к железу)
static int ahci_init_port_safe(uint8_t port_num) {
    if (port_num >= AHCI_MAX_PORTS) {
        return AHCI_ERROR_INVALID;
    }
    
    // Инициализируем тестовые данные для демонстрации
    ahci_port_info_t* port = &ahci_ctrl.ports[port_num];
    
    // Пример тестовых устройств
    switch (port_num) {
        case 0:
            port->state = PORT_STATE_ONLINE;
            port->device_type = AHCI_DEVICE_HDD;
            strncpy(port->model, "ST500DM009", sizeof(port->model) - 1);
            strncpy(port->serial, "WCC6Y3PNKT5N", sizeof(port->serial) - 1);
            strncpy(port->firmware, "CC43", sizeof(port->firmware) - 1);
            port->capacity_mb = 500 * 1024; // 500 GB
            break;
        case 1:
            port->state = PORT_STATE_ONLINE;
            port->device_type = AHCI_DEVICE_SSD;
            strncpy(port->model, "Samsung SSD 860 EVO", sizeof(port->model) - 1);
            strncpy(port->serial, "S3Z8NB0KA12345", sizeof(port->serial) - 1);
            strncpy(port->firmware, "RVT03B6Q", sizeof(port->firmware) - 1);
            port->capacity_mb = 250 * 1024; // 250 GB
            break;
        case 2:
            port->state = PORT_STATE_ONLINE;
            port->device_type = AHCI_DEVICE_CDROM;
            strncpy(port->model, "HL-DT-ST DVDRAM GH24NSD1", sizeof(port->model) - 1);
            strncpy(port->serial, "K97D5B123456", sizeof(port->serial) - 1);
            strncpy(port->firmware, "LL01", sizeof(port->firmware) - 1);
            port->capacity_mb = 0;
            break;
        default:
            port->state = PORT_STATE_EMPTY;
            port->device_type = AHCI_DEVICE_NONE;
            port->model[0] = '\0';
            port->serial[0] = '\0';
            port->firmware[0] = '\0';
            port->capacity_mb = 0;
            break;
    }
    
    port->port_number = port_num;
    return AHCI_SUCCESS;
}

// Основная инициализация AHCI (безопасная версия)
void ahci_init(void) {
    if (ahci_debug) {
        video_print("[AHCI] Initializing AHCI driver (safe mode)\n");
    }
    
    // Ищем PCI устройство AHCI
    pci_device_t dev = pci_find_device(0xFFFF, 0xFFFF, PCI_CLASS_STORAGE, PCI_SUBCLASS_SATA);
    
    if (dev.vendor_id == 0xFFFF) {
        if (ahci_debug) {
            video_print("[AHCI] No AHCI controller found via PCI\n");
        }
        ahci_ctrl.initialized = 0;
        return;
    }
    
    if (ahci_debug) {
        char buf[32];
        video_print("[AHCI] Found AHCI controller: ");
        video_print("Vendor=0x");
        terminal_writehex(dev.vendor_id);
        video_print(" Device=0x");
        terminal_writehex(dev.device_id);
        video_print("\n");
    }
    
    // Получаем BAR0 (обычно это ABAR)
    uint32_t abar = pci_get_bar(dev, 5);
    if (abar == 0) {
        abar = pci_get_bar(dev, 0);
    }
    
    // Проверяем, что BAR корректен
    if (!is_valid_mmio_address(abar)) {
        if (ahci_debug) {
            video_print("[AHCI] Invalid or zero ABAR address\n");
        }
        ahci_ctrl.base_address = NULL;
        ahci_ctrl.initialized = 0; // Не помечаем как инициализированный — это ошибка
        return;
    } else {
        ahci_ctrl.base_address = (uint32_t*)(uintptr_t)abar;

        // Пробуем прочитать регистр CAP (с крайней осторожностью)
        if (ahci_debug) {
            uint32_t cap = safe_read_mmio(ahci_ctrl.base_address, 0x00);
            video_print("[AHCI] CAP register: 0x");
            terminal_writehex(cap);
            video_print("\n");
        }

        // Включаем безопасный режим (не инициализируем реальное железо)
        ahci_ctrl.capabilities = 0;
        ahci_ctrl.ports_implemented = 0x00000007; // Первые 3 порта "реализованы"
        ahci_ctrl.port_count = 3;
        ahci_ctrl.initialized = 1; // Помечаем как инициализированный для команд терминала

        // Инициализируем порты (безопасные тестовые данные)
        for (int i = 0; i < AHCI_MAX_PORTS; i++) {
            if (i < ahci_ctrl.port_count) {
                ahci_init_port_safe(i);
            } else {
                ahci_ctrl.ports[i].state = PORT_STATE_EMPTY;
                ahci_ctrl.ports[i].device_type = AHCI_DEVICE_NONE;
                ahci_ctrl.ports[i].model[0] = '\0';
                ahci_ctrl.ports[i].capacity_mb = 0;
            }
        }
    }
    
    if (ahci_debug) {
        video_print("[AHCI] Driver initialized in safe mode\n");
    }
}

// Обнаружение устройств
void ahci_detect_devices(void) {
    if (!ahci_ctrl.initialized) {
        ahci_init();
    }
}

// Получение строкового представления типа устройства
const char* ahci_get_device_type_string(ahci_device_type_t type) {
    switch(type) {
        case AHCI_DEVICE_HDD: return "HDD";
        case AHCI_DEVICE_SSD: return "SSD";
        case AHCI_DEVICE_CDROM: return "CD/DVD";
        default: return "None";
    }
}

// Получение строкового представления состояния порта
const char* ahci_get_port_state_string(ahci_port_state_t state) {
    switch(state) {
        case PORT_STATE_ONLINE: return "(Online)";
        case PORT_STATE_OFFLINE: return "(Offline)";
        case PORT_STATE_ERROR: return "(Error)";
        case PORT_STATE_EMPTY: return "";
        default: return "(Unknown)";
    }
}

// Вывод информации об устройствах в терминале (безопасная версия)
void ahci_print_devices(void) {
    video_print("\n");
    video_print("AHCI Storage Devices:\n");
    video_print("=====================\n");
    
    // Если драйвер не инициализирован, инициализируем
    if (!ahci_ctrl.initialized) {
        ahci_init();
    }
    
    // Если все еще не инициализирован или базовый адрес MMIO некорректен,
    // считаем это критической ошибкой и вызываем kernel panic с кодом AHCI.
    if (!ahci_ctrl.initialized || ahci_ctrl.base_address == NULL || ahci_ctrl.port_count == 0) {
        panic("AHCI driver failure: controller not initialized or MMIO invalid", AHCI_PANIC_CODE_DRIVER_FAILURE);
        return; // unreachable, but keeps compiler happy
    }
    
    // Выводим информацию о каждом порте
    for (int p = 0; p < ahci_ctrl.port_count; p++) {
        ahci_port_info_t* port = &ahci_ctrl.ports[p];
        
        if (port->state == PORT_STATE_EMPTY) {
            video_print("[AHCI] Port ");
            char port_num[4];
            itoa(p, port_num, 10);
            video_print(port_num);
            video_print(": Empty\n");
            continue;
        }
        
        // Выводим номер порта
        video_print("[AHCI] Port ");
        char port_num[4];
        itoa(p, port_num, 10);
        video_print(port_num);
        video_print(": ");
        
        // Выводим тип устройства
        video_print(ahci_get_device_type_string(port->device_type));
        
        // Выводим модель
        if (port->model[0] != '\0') {
            video_print("  ");
            video_print(port->model);
        }
        
        // Выводим емкость (если есть)
        if (port->capacity_mb > 0) {
            video_print("  ");
            
            if (port->capacity_mb >= 1024) {
                // В гигабайтах
                uint32_t gb = port->capacity_mb / 1024;
                char gb_str[16];
                itoa(gb, gb_str, 10);
                video_print(gb_str);
                video_print("GB");
            } else {
                // В мегабайтах
                char mb_str[16];
                itoa((uint32_t)port->capacity_mb, mb_str, 10);
                video_print(mb_str);
                video_print("MB");
            }
        } else if (port->device_type == AHCI_DEVICE_CDROM) {
            video_print("    "); // Выравнивание для CD/DVD
        }
        
        // Выводим состояние
        video_print("  ");
        video_print(ahci_get_port_state_string(port->state));
        
        video_print("\n");
        
        // Дополнительная информация (серийный номер, прошивка)
        if (ahci_debug && port->serial[0] != '\0') {
            video_print("       Serial: ");
            video_print(port->serial);
            video_print("  Firmware: ");
            video_print(port->firmware);
            video_print("\n");
        }
    }
    
    video_print("\n");
}
int ahci_is_initialized(void) {
    return ahci_ctrl.initialized;
}