#include "gui.h"
#include "gfx.h"
#include "gfx_window.h"
#include "keyboard.h"
#include "video.h"  // Добавлен этот include

void gui_handle_input() {
    char key = keyboard_getchar();
    
    switch(key) {
        case 0x3B:
            wm_create_terminal_window();
            break;
        case 0x01:
            gui_mode = 0;
            break;
    }
}

void gui_init() {
    gfx_init();
    wm_init();
    gui_mode = 1;
    wm_redraw_all();
}

void gui_run() {
    gui_init();
    
    while(gui_mode) {
        gui_handle_input();
        wm_redraw_all();
        
        // Более эффективная задержка
        for(volatile int i = 0; i < 50000; i++) {
            asm volatile ("nop");
        }
    }
    
    // Возвращаемся в текстовый режим
    video_switch_to_text();
    video_clear();  // Очищаем экран
}