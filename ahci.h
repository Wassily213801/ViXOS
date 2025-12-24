#ifndef AHCI_H
#define AHCI_H

#include <stdint.h>

// Константы AHCI
#define AHCI_MAX_PORTS 32
#define AHCI_SECTOR_SIZE 512

// Коды ошибок
typedef enum {
    AHCI_SUCCESS = 0,
    AHCI_ERROR_NO_CONTROLLER = -1,
    AHCI_ERROR_PORT_INIT = -2,
    AHCI_ERROR_DEVICE = -3,
    AHCI_ERROR_TIMEOUT = -4,
    AHCI_ERROR_INVALID = -5
} ahci_error_t;

// Типы устройств
typedef enum {
    AHCI_DEVICE_NONE = 0,
    AHCI_DEVICE_HDD = 1,      // Жесткий диск
    AHCI_DEVICE_SSD = 2,      // SSD накопитель
    AHCI_DEVICE_CDROM = 3     // CD/DVD привод
} ahci_device_type_t;

// Состояние порта
typedef enum {
    PORT_STATE_EMPTY = 0,
    PORT_STATE_ONLINE = 1,
    PORT_STATE_OFFLINE = 2,
    PORT_STATE_ERROR = 3
} ahci_port_state_t;

// Структура порта (упрощенная)
typedef struct {
    uint8_t port_number;
    ahci_device_type_t device_type;
    ahci_port_state_t state;
    char model[41];           // Модель устройства
    uint64_t capacity_mb;     // Емкость в МБ
    char serial[21];          // Серийный номер
    char firmware[9];         // Версия прошивки
} ahci_port_info_t;

// Структура контроллера
typedef struct {
    uint32_t* base_address;   // Базовый адрес ABAR
    uint32_t capabilities;    // Возможности контроллера
    uint32_t ports_implemented; // Реализованные порты
    uint8_t port_count;       // Количество портов
    ahci_port_info_t ports[AHCI_MAX_PORTS]; // Информация о портах
    uint8_t initialized;      // Флаг инициализации
} ahci_controller_t;

// Прототипы функций
void ahci_init(void);
void ahci_detect_devices(void);
void ahci_print_devices(void);
const char* ahci_get_device_type_string(ahci_device_type_t type);
const char* ahci_get_port_state_string(ahci_port_state_t state);

// Управление отладочным выводом AHCI модуля
void ahci_set_debug(int on);
extern int ahci_debug;

// Глобальный экземпляр контроллера
extern ahci_controller_t ahci_ctrl;
int ahci_is_initialized(void);
#endif // AHCI_H