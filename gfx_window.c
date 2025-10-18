#include "gfx_window.h"
#include "gfx.h"
#include "video.h"
#include "pmm.h"
#include <string.h>
#define TASKBAR_HEIGHT 30
#define WINDOW_TITLE_HEIGHT 20
#define WINDOW_BORDER_SIZE 2

static WindowManager wm;

void wm_init() {
    wm.window_count = 0;
    wm.active_window = -1;
}

int wm_create_window(int x, int y, int width, int height, const char* title) {
    if (wm.window_count >= MAX_WINDOWS) return -1;
    
    Window* win = &wm.windows[wm.window_count];
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    strncpy(win->title, title, 31);
    win->title[31] = '\0';
    win->color = GFX_COLOR_GRAY;
    win->buffer = (uint8_t*)pmm_alloc_block();
    win->visible = 1;
    
    return wm.window_count++;
}

void wm_destroy_window(int window_id) {
    if (window_id < 0 || window_id >= wm.window_count) return;
    
    pmm_free_block(wm.windows[window_id].buffer);
    
    for (int i = window_id; i < wm.window_count - 1; i++) {
        wm.windows[i] = wm.windows[i + 1];
    }
    
    wm.window_count--;
}

void wm_draw_window(int window_id) {
    if (window_id < 0 || window_id >= wm.window_count) return;
    
    Window* win = &wm.windows[window_id];
    if (!win->visible) return;
    
    // Draw window border
    gfx_draw_rect(win->x, win->y, win->width, win->height, GFX_COLOR_BLACK);
    
    // Draw title bar
    gfx_fill_rect(win->x + 1, win->y + 1, win->width - 2, WINDOW_TITLE_HEIGHT, 
                 (window_id == wm.active_window) ? GFX_COLOR_BLUE : GFX_COLOR_DARK_GRAY);
    
    // Draw title
    gfx_draw_string(win->x + 5, win->y + 5, win->title, GFX_COLOR_WHITE);
    
    // Draw close button
    gfx_draw_rect(win->x + win->width - 20, win->y + 5, 15, 10, GFX_COLOR_BLACK);
    gfx_draw_string(win->x + win->width - 17, win->y + 5, "X", GFX_COLOR_WHITE);
    
    // Draw content area
    gfx_fill_rect(win->x + 1, win->y + WINDOW_TITLE_HEIGHT + 1, 
                 win->width - 2, win->height - WINDOW_TITLE_HEIGHT - 2, 
                 GFX_COLOR_GRAY);
}

void wm_draw_taskbar() {
    int width = gfx_ctx.width;
    int height = gfx_ctx.height;
    
    // Draw taskbar background
    gfx_fill_rect(0, height - TASKBAR_HEIGHT, width, TASKBAR_HEIGHT, GFX_COLOR_DARK_GRAY);
    
    // Draw start button placeholder
    gfx_fill_rect(5, height - TASKBAR_HEIGHT + 5, 60, TASKBAR_HEIGHT - 10, GFX_COLOR_GRAY);
    gfx_draw_string(10, height - TASKBAR_HEIGHT + 10, "Terminal", GFX_COLOR_BLACK);
    
    // Draw clock placeholder
    gfx_fill_rect(width - 100, height - TASKBAR_HEIGHT + 5, 95, TASKBAR_HEIGHT - 10, GFX_COLOR_GRAY);
    gfx_draw_string(width - 95, height - TASKBAR_HEIGHT + 10, "00:00", GFX_COLOR_BLACK);
}

void wm_redraw_all() {
    // Draw desktop background
    gfx_clear_screen(GFX_COLOR_BLUE);
    
    // Draw desktop icons
    gfx_draw_string(20, 20, "Terminal [F1]", GFX_COLOR_WHITE);
    gfx_draw_string(20, 40, "Exit GUI [ESC]", GFX_COLOR_WHITE);
    
    // Draw all windows
    for (int i = 0; i < wm.window_count; i++) {
        wm_draw_window(i);
    }
    
    // Draw taskbar
    wm_draw_taskbar();
    
    gfx_update();
}

void wm_set_active(int window_id) {
    if (window_id >= 0 && window_id < wm.window_count) {
        wm.active_window = window_id;
    }
}

void wm_move_window(int window_id, int x, int y) {
    if (window_id >= 0 && window_id < wm.window_count) {
        wm.windows[window_id].x = x;
        wm.windows[window_id].y = y;
    }
}

// Terminal window implementation
static int terminal_window_id = -1;

void wm_create_terminal_window() {
    terminal_window_id = wm_create_window(100, 100, 400, 300, "Terminal");
    wm_set_active(terminal_window_id);
}

void wm_terminal_putchar(char c) {
    if (terminal_window_id == -1) return;
    
    Window* win = &wm.windows[terminal_window_id];
    static int x = 5, y = WINDOW_TITLE_HEIGHT + 5;
    
    if (c == '\n') {
        x = 5;
        y += 8;
        if (y > win->height - 15) {
            y = WINDOW_TITLE_HEIGHT + 5;
            gfx_fill_rect(win->x + 1, win->y + WINDOW_TITLE_HEIGHT + 1, 
                         win->width - 2, win->height - WINDOW_TITLE_HEIGHT - 2, 
                         GFX_COLOR_GRAY);
        }
        return;
    }
    
    gfx_draw_char(win->x + x, win->y + y, c, GFX_COLOR_BLACK);
    x += 8;
    
    if (x > win->width - 10) {
        x = 5;
        y += 8;
    }
}

void wm_terminal_writestring(const char* str) {
    while (*str) {
        wm_terminal_putchar(*str++);
    }
}