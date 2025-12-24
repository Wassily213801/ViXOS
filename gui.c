// gui.c - МАКСИМАЛЬНО УПРОЩЕННЫЙ ВАРИАНТ
#include "gui.h"
#include "gfx.h"
#include "video.h"
#include "keyboard.h"

void gui_init() {
    video_init_graphics();
    video_switch_to_graphics();
    gfx_init();
    
    // Просто голубой экран без всего
    gfx_clear_screen(GFX_COLOR_LIGHT_BLUE);
    gfx_update();
}

void gui_run() {
    gui_init();
    
    // Просто ждем ESC
    while (1) {
        char key = keyboard_getchar();
        if (key == 0x1B) { // ESC
            break;
        }
    }
    
    // Возврат в текстовый режим
    video_switch_to_text();
    video_clear();
}

// УДАЛИТЬ функцию gui_command() отсюда
// Она уже определена в terminal.c