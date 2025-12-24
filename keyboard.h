#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

// Коды специальных клавиш
#define KEY_ESC        0x1B
#define KEY_BACKSPACE  '\b'
#define KEY_TAB        '\t'
#define KEY_ENTER      '\n'
#define KEY_UP         0x80
#define KEY_DOWN       0x81
#define KEY_LEFT       0x82
#define KEY_RIGHT      0x83
#define KEY_F1         0x84
#define KEY_F2         0x85
#define KEY_F3         0x86
#define KEY_F4         0x87
#define KEY_F5         0x88
#define KEY_F6         0x89
#define KEY_F7         0x8A
#define KEY_F8         0x8B
#define KEY_F9         0x8C
#define KEY_F10        0x8D
#define KEY_F11        0x8E
#define KEY_F12        0x8F
#define KEY_INSERT     0x90
#define KEY_DELETE     0x91
#define KEY_HOME       0x92
#define KEY_END        0x93
#define KEY_PAGEUP     0x94
#define KEY_PAGEDOWN   0x95

void keyboard_init(void);
int keyboard_getkey(void);
char keyboard_getchar(void);
char keyboard_getchar_noblock(void);  // Добавлена эта строка
int keyboard_getkey_noblock(void);
int keyboard_is_special(int key);
const char* keyboard_get_key_name(int key);

#endif