#include "keyboard.h"
#include "port_io.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

static const char keymap[] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'', '`', 0, '\\',
    'z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ', 0
};

void keyboard_init() {
    // Пока не требуется инициализация
}

char keyboard_getchar() {
    uint8_t scancode = 0;

    while (1) {
        if (inb(KEYBOARD_STATUS_PORT) & 1) {
            scancode = inb(KEYBOARD_DATA_PORT);
            if (!(scancode & 0x80)) {  // пропуск отпускания клавиш
                if (scancode < sizeof(keymap)) {
                    return keymap[scancode];
                }
            }
        }
    }

    return 0;
}