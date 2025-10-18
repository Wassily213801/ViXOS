#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

// Только extern объявление, без определения
extern int gui_mode;

void terminal_init();
void terminal_run();
void terminal_draw(int x, int y);
void terminal_putchar(char c);
void terminal_writestring(const char* data);
void terminal_putchar(char c);  // Добавьте эту строку
void terminal_puts(const char* str);
void gui_command();
void terminal_writehex(uint32_t value);
void guess_game_integrated(void); // Оставьте только это объявление
void time_command(); // Добавьте эту строку
#endif