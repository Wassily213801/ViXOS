// gui.c - МАКСИМАЛЬНО ПРОСТОЙ ГРАФИЧЕСКИЙ ИНТЕРФЕЙС ViXOS
#include "gui.h"
#include "gfx.h"
#include "video.h"
#include "keyboard.h"
#include "serial.h"
#include "string.h"

// Определение глобальной переменной - ТОЛЬКО ЗДЕСЬ
int gui_mode = 0;

void gui_init() {
    video_print("[GUI] init: calling video_init_graphics()\n");
    serial_write("[GUI] init: calling video_init_graphics()\n");
    video_init_graphics();
    video_print("[GUI] init: calling video_switch_to_graphics()\n");
    serial_write("[GUI] init: calling video_switch_to_graphics()\n");
    video_switch_to_graphics();
    gfx_init();
    video_print("[GUI] init: gfx_init() done\n");
    serial_write("[GUI] init: gfx_init() done\n");
    
    // Полностью синий рабочий стол
    gfx_clear_screen(GFX_COLOR_BLUE);
    
    // Основной текст в центре экрана
    int screen_width = video_get_width();
    int screen_height = video_get_height();
    
    const char* main_text = "This graphical interface is still in the development stages.";
    int text_x = (screen_width - strlen(main_text) * 4) / 2;
    int text_y = screen_height / 2 - 20;
    
    gfx_draw_string(text_x, text_y, main_text, GFX_COLOR_WHITE);
    
    // Информация о версии в правом нижнем углу
    const char* version_lines[] = {
        "ViXOS - Code Name: Nova",
        "Build: 37.27",
        "Developer Edition Only",
        "Press ESC to exit"
    };
    
    int line_height = 8;
    int start_y = screen_height - (4 * line_height) - 10;
    
    for (int i = 0; i < 4; i++) {
        int line_x = screen_width - strlen(version_lines[i]) * 4 - 20;
        gfx_draw_string(line_x, start_y + (i * line_height), 
                       version_lines[i], GFX_COLOR_LIGHT_GRAY);
    }
    
    // Обновляем экран
    gfx_update();
}

void gui_run() {
    gui_mode = 1;
    gui_init();
    
    // Ожидание нажатия ESC
    video_print("[GUI] run: waiting for ESC...\n");
    serial_write("[GUI] run: waiting for ESC...\n");
    while (1) {
        char key = keyboard_getchar();
        if (key == 0x1B) { // ESC
            break;
        }
    }
    
    // Возврат в текстовый режим
    gui_mode = 0;
    video_print("[GUI] exiting: switching to text mode\n");
    serial_write("[GUI] exiting: switching to text mode\n");
    // Безопасный возврат в текстовый режим (без BIOS int в защищённом режиме)
    video_switch_to_text();
    video_clear();
}

// Определение функции - ТОЛЬКО ЗДЕСЬ
void gui_command() {
    gui_run();
}