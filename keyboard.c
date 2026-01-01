#include "keyboard.h"
#include "port_io.h"

static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int alt_pressed = 0;
static int caps_lock = 0;
static int num_lock = 1;
static int key_released = 1;

// Буфер для хранения нажатых клавиш
#define KEYBOARD_BUFFER_SIZE 32
static unsigned char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static int buffer_start = 0;
static int buffer_end = 0;
static int buffer_count = 0;

// Обычная раскладка
static const char keymap[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\',
    'z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Shift раскладка
static const char shift_map[128] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
    'A','S','D','F','G','H','J','K','L',':','\"','~', 0,'|',
    'Z','X','C','V','B','N','M','<','>','?', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Добавляем символ в буфер
static void keyboard_buffer_put(unsigned char c) {
    if (buffer_count < KEYBOARD_BUFFER_SIZE) {
        keyboard_buffer[buffer_end] = c;
        buffer_end = (buffer_end + 1) % KEYBOARD_BUFFER_SIZE;
        buffer_count++;
    }
}

// Берем символ из буфера
static int keyboard_buffer_get(void) {
    if (buffer_count > 0) {
        unsigned char c = keyboard_buffer[buffer_start];
        buffer_start = (buffer_start + 1) % KEYBOARD_BUFFER_SIZE;
        buffer_count--;
        return (int)c;
    }
    return 0;
}

void keyboard_init(void) {
    shift_pressed = 0;
    ctrl_pressed = 0;
    alt_pressed = 0;
    caps_lock = 0;
    key_released = 1;
    buffer_start = 0;
    buffer_end = 0;
    buffer_count = 0;
}

// Обработка сканкода и добавление в буфер
static void process_scancode(uint8_t scancode) {
    int extended = 0;

    // Обработка E0-префикса: если пришел 0xE0, читаем следующий байт и помечаем как расширенный
    if (scancode == 0xE0) {
        // Ждем следующий байт
        while (!(inb(0x64) & 0x01)) { }
        scancode = inb(0x60);
        extended = 1;
    }

    if (scancode & 0x80) {
        // Отпускание клавиши
        scancode &= 0x7F;

        switch (scancode) {
            case 0x2A: case 0x36: // Left/Right Shift
                shift_pressed = 0;
                break;
            case 0x1D: // Ctrl (левый или правый)
                ctrl_pressed = 0;
                break;
            case 0x38: // Alt (левый или правый)
                alt_pressed = 0;
                break;
        }
        return;
    }

    // Нажатие клавиши: обработка модификаторов
    switch (scancode) {
        case 0x2A: case 0x36: // Left/Right Shift
            shift_pressed = 1;
            return;
        case 0x1D: // Ctrl
            ctrl_pressed = 1;
            return;
        case 0x38: // Alt
            alt_pressed = 1;
            return;
        case 0x3A: // Caps Lock
            caps_lock = !caps_lock;
            return;
        case 0x45: // Num Lock
            num_lock = !num_lock;
            return;
    }

    // Если расширенный (E0) — маппим стрелки и навигацию
    if (extended) {
        int special_key = 0;
        switch (scancode) {
            case 0x48: special_key = KEY_UP; break;
            case 0x50: special_key = KEY_DOWN; break;
            case 0x4B: special_key = KEY_LEFT; break;
            case 0x4D: special_key = KEY_RIGHT; break;
            case 0x52: special_key = KEY_INSERT; break;
            case 0x53: special_key = KEY_DELETE; break;
            case 0x47: special_key = KEY_HOME; break;
            case 0x4F: special_key = KEY_END; break;
            case 0x49: special_key = KEY_PAGEUP; break;
            case 0x51: special_key = KEY_PAGEDOWN; break;
        }
        if (special_key) {
            keyboard_buffer_put((unsigned char)special_key);
        }
        return;
    }

    // Обработка цифрового блока (без E0). Если Num Lock включен — выдаем цифры/знаки,
    // иначе — навигацию (HOME/END/PGUP/PGDN/INS/DEL/стрелки)
    switch (scancode) {
        case 0x47: // Numpad 7 / Home
            if (num_lock) { keyboard_buffer_put('7'); return; } else { keyboard_buffer_put((unsigned char)KEY_HOME); return; }
        case 0x48: // Numpad 8 / Up
            if (num_lock) { keyboard_buffer_put('8'); return; } else { keyboard_buffer_put((unsigned char)KEY_UP); return; }
        case 0x49: // Numpad 9 / PageUp
            if (num_lock) { keyboard_buffer_put('9'); return; } else { keyboard_buffer_put((unsigned char)KEY_PAGEUP); return; }
        case 0x4B: // Numpad 4 / Left
            if (num_lock) { keyboard_buffer_put('4'); return; } else { keyboard_buffer_put((unsigned char)KEY_LEFT); return; }
        case 0x4C: // Numpad 5
            if (num_lock) { keyboard_buffer_put('5'); return; } else { return; }
        case 0x4D: // Numpad 6 / Right
            if (num_lock) { keyboard_buffer_put('6'); return; } else { keyboard_buffer_put((unsigned char)KEY_RIGHT); return; }
        case 0x4F: // Numpad 1 / End
            if (num_lock) { keyboard_buffer_put('1'); return; } else { keyboard_buffer_put((unsigned char)KEY_END); return; }
        case 0x50: // Numpad 2 / Down
            if (num_lock) { keyboard_buffer_put('2'); return; } else { keyboard_buffer_put((unsigned char)KEY_DOWN); return; }
        case 0x51: // Numpad 3 / PageDown
            if (num_lock) { keyboard_buffer_put('3'); return; } else { keyboard_buffer_put((unsigned char)KEY_PAGEDOWN); return; }
        case 0x52: // Numpad 0 / Insert
            if (num_lock) { keyboard_buffer_put('0'); return; } else { keyboard_buffer_put((unsigned char)KEY_INSERT); return; }
        case 0x53: // Numpad . / Delete
            if (num_lock) { keyboard_buffer_put('.'); return; } else { keyboard_buffer_put((unsigned char)KEY_DELETE); return; }
        case 0x35: // Numpad / (обычно E0 35 на некоторых клавиатурах)
            keyboard_buffer_put('/'); return;
        case 0x37: // Numpad *
            keyboard_buffer_put('*'); return;
        case 0x4A: // Numpad -
            keyboard_buffer_put('-'); return;
        case 0x4E: // Numpad +
            keyboard_buffer_put('+'); return;
    }

    // Обработка функциональных клавиш F1-F12 и других стандартных специальных клавиш
    int special_key = 0;
    switch (scancode) {
        case 0x3B: special_key = KEY_F1; break;
        case 0x3C: special_key = KEY_F2; break;
        case 0x3D: special_key = KEY_F3; break;
        case 0x3E: special_key = KEY_F4; break;
        case 0x3F: special_key = KEY_F5; break;
        case 0x40: special_key = KEY_F6; break;
        case 0x41: special_key = KEY_F7; break;
        case 0x42: special_key = KEY_F8; break;
        case 0x43: special_key = KEY_F9; break;
        case 0x44: special_key = KEY_F10; break;
        case 0x57: special_key = KEY_F11; break;
        case 0x58: special_key = KEY_F12; break;
    }
        if (special_key) {
            keyboard_buffer_put((unsigned char)special_key);
            return;
        }

    // Обработка обычных символов
    char key;
    if (shift_pressed) {
        key = shift_map[scancode];
    } else {
        key = keymap[scancode];
    }

    // Обработка Caps Lock
    if (caps_lock && !shift_pressed) {
        if (key >= 'a' && key <= 'z') {
            key -= 32; // Преобразовать в верхний регистр
        } else if (key >= 'A' && key <= 'Z') {
            key += 32; // Преобразовать в нижний регистр
        }
    }

    // Если клавиша действительна, добавляем в буфер
    if (key != 0) {
        keyboard_buffer_put((unsigned char)key);
    }
}

int keyboard_getkey(void) {
    while (buffer_count == 0) {
        // Ждем данные от клавиатуры
        while (!(inb(0x64) & 0x01)) {
            // Пустая петля ожидания
        }
        
        uint8_t scancode = inb(0x60);
        process_scancode(scancode);
    }
    
    return keyboard_buffer_get();
}

char keyboard_getchar(void) {
    int key;
    do {
        key = keyboard_getkey();
    } while (key >= 0x80); // Пропускаем специальные клавиши
    
    return (char)key;
}

char keyboard_getchar_noblock(void) {
    // Сначала проверяем буфер
    if (buffer_count > 0) {
        int c = keyboard_buffer_get();
        // Пропускаем специальные клавиши в non-blocking режиме
        if (c >= 0x80) {
            return 0;
        }
        return (char)c;
    }
    
    // Проверяем, есть ли новые данные от клавиатуры
    if (inb(0x64) & 0x01) {
        uint8_t scancode = inb(0x60);
        process_scancode(scancode);
        
        // Если что-то добавилось в буфер, возвращаем (только обычные символы)
        if (buffer_count > 0) {
            int c = keyboard_buffer_get();
            if (c >= 0x80) {
                return 0;
            }
            return (char)c;
        }
    }
    
    return 0;
}

int keyboard_getkey_noblock(void) {
    // Если есть данные в буфере — возвращаем
    if (buffer_count > 0) {
        return keyboard_buffer_get();
    }

    // Если есть данные в порту — обработаем и вернём, иначе 0
    if (inb(0x64) & 0x01) {
        uint8_t scancode = inb(0x60);
        process_scancode(scancode);

        if (buffer_count > 0) {
            return keyboard_buffer_get();
        }
    }

    return 0;
}

int keyboard_is_special(int key) {
    return key >= 0x80;
}

const char* keyboard_get_key_name(int key) {
    switch (key) {
        case KEY_ESC:       return "ESC";
        case KEY_UP:        return "UP";
        case KEY_DOWN:      return "DOWN";
        case KEY_LEFT:      return "LEFT";
        case KEY_RIGHT:     return "RIGHT";
        case KEY_F1:        return "F1";
        case KEY_F2:        return "F2";
        case KEY_F3:        return "F3";
        case KEY_F4:        return "F4";
        case KEY_F5:        return "F5";
        case KEY_F6:        return "F6";
        case KEY_F7:        return "F7";
        case KEY_F8:        return "F8";
        case KEY_F9:        return "F9";
        case KEY_F10:       return "F10";
        case KEY_F11:       return "F11";
        case KEY_F12:       return "F12";
        case KEY_INSERT:    return "INSERT";
        case KEY_DELETE:    return "DELETE";
        case KEY_HOME:      return "HOME";
        case KEY_END:       return "END";
        case KEY_PAGEUP:    return "PAGEUP";
        case KEY_PAGEDOWN:  return "PAGEDOWN";
        default:            return "UNKNOWN";
    }
}
static int get_game_input(void) {
    // Простой опрос порта клавиатуры
    if (!(inb(0x64) & 0x01)) {
        return 0; // Нет данных
    }
    
    uint8_t scancode = inb(0x60);
    
    // Обрабатываем только нажатия (не отпускания)
    if (scancode & 0x80) {
        return 0;
    }
    
    // Обработка основных игровых клавиш
    switch (scancode) {
        case 0x11: return 'w'; // W
        case 0x1F: return 's'; // S
        case 0x1E: return 'a'; // A
        case 0x20: return 'd'; // D
        case 0x39: return ' '; // Space
        case 0x01: return 0x1B; // ESC
        case 0x48: return KEY_UP;
        case 0x50: return KEY_DOWN;
        case 0x4B: return KEY_LEFT;
        case 0x4D: return KEY_RIGHT;
        default: return 0;
    }
}