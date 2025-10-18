#ifndef GFX_WINDOW_H
#define GFX_WINDOW_H

#include <stdint.h>

#define MAX_WINDOWS 10

typedef struct {
    int x, y;
    int width, height;
    char title[32];
    uint8_t color;
    uint8_t* buffer;
    int visible;
} Window;

typedef struct {
    Window windows[MAX_WINDOWS];
    int window_count;
    int active_window;
} WindowManager;

// Window management
void wm_init();
int wm_create_window(int x, int y, int width, int height, const char* title);
void wm_destroy_window(int window_id);
void wm_draw_window(int window_id);
void wm_redraw_all();
void wm_set_active(int window_id);
void wm_move_window(int window_id, int x, int y);

// Taskbar
void wm_draw_taskbar();

// Terminal window specific
void wm_create_terminal_window();
void wm_terminal_putchar(char c);
void wm_terminal_writestring(const char* str);

#endif