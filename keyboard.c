#include "keyboard.h"
#include "port_io.h"

static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int alt_pressed = 0;
static int caps_lock = 0;
static int key_released = 1;

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

void keyboard_init(void) {
    shift_pressed = 0;
    ctrl_pressed = 0;
    alt_pressed = 0;
    caps_lock = 0;
    key_released = 1;
}

int keyboard_getkey(void) {
    while (1) {
        uint8_t scancode = inb(0x60);

        if (scancode & 0x80) {
            // Отпускание клавиши
            scancode &= 0x7F;
            
            switch (scancode) {
                case 0x2A: case 0x36: // Left/Right Shift
                    shift_pressed = 0;
                    break;
                case 0x1D: // Ctrl
                    ctrl_pressed = 0;
                    break;
                case 0x38: // Alt
                    alt_pressed = 0;
                    break;
            }
            
            key_released = 1;
        } else {
            // Нажатие клавиши
            if (!key_released) continue; // ждём отпускания предыдущей
            key_released = 0;

            // Обработка модификаторов
            switch (scancode) {
                case 0x2A: case 0x36: // Left/Right Shift
                    shift_pressed = 1;
                    continue;
                case 0x1D: // Ctrl
                    ctrl_pressed = 1;
                    continue;
                case 0x38: // Alt
                    alt_pressed = 1;
                    continue;
                case 0x3A: // Caps Lock
                    caps_lock = !caps_lock;
                    continue;
            }

            // Обработка E0-префиксных клавиш (расширенные клавиши)
            if (scancode == 0xE0) {
                // Это E0-префикс, читаем следующий скан-код
                scancode = inb(0x60);
                
                switch (scancode) {
                    case 0x48: return KEY_UP;      // Up arrow
                    case 0x50: return KEY_DOWN;    // Down arrow
                    case 0x4B: return KEY_LEFT;    // Left arrow
                    case 0x4D: return KEY_RIGHT;   // Right arrow
                    case 0x52: return KEY_INSERT;  // Insert
                    case 0x53: return KEY_DELETE;  // Delete
                    case 0x47: return KEY_HOME;    // Home
                    case 0x4F: return KEY_END;     // End
                    case 0x49: return KEY_PAGEUP;  // Page Up
                    case 0x51: return KEY_PAGEDOWN;// Page Down
                }
                continue;
            }

            // Обработка функциональных клавиш F1-F12
            switch (scancode) {
                case 0x3B: return KEY_F1;      // F1
                case 0x3C: return KEY_F2;      // F2
                case 0x3D: return KEY_F3;      // F3
                case 0x3E: return KEY_F4;      // F4
                case 0x3F: return KEY_F5;      // F5
                case 0x40: return KEY_F6;      // F6
                case 0x41: return KEY_F7;      // F7
                case 0x42: return KEY_F8;      // F8
                case 0x43: return KEY_F9;      // F9
                case 0x44: return KEY_F10;     // F10
                case 0x57: return KEY_F11;     // F11
                case 0x58: return KEY_F12;     // F12
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

            // Если клавиша действительна, вернуть её
            if (key != 0) {
                return (int)key;
            }
        }
    }
}

char keyboard_getchar(void) {
    int key;
    do {
        key = keyboard_getkey();
    } while (key > 0xFF); // Пропускаем специальные клавиши
    
    return (char)key;
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