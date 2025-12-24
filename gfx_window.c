// gfx_window.c (улучшенная версия с исправлениями)
#include "gfx_window.h"
#include "gfx.h"
#include "video.h"
#include "pmm.h"
#include "keyboard.h"
#include "string.h"

WindowManager wm;

void wm_init() {
    wm.window_count = 0;
    wm.active_window = -1;
    wm.screen_width = video_get_width();
    wm.screen_height = video_get_height();
}

int wm_create_window(int x, int y, int width, int height, const char* title) {
    if (wm.window_count >= MAX_WINDOWS) return -1;
    
    Window* win = &wm.windows[wm.window_count];
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    
    // Simple string copy
    const char* src = title;
    char* dst = win->title;
    int i = 0;
    while (*src && i < 31) {
        *dst++ = *src++;
        i++;
    }
    *dst = '\0';
    
    // Выделяем память для буфера окна
    win->buffer = (uint8_t*)pmm_alloc_block();
    if (!win->buffer) return -1; // Проверка выделения памяти
    
    win->visible = 1;
    win->active = 0;
    win->needs_redraw = 1;
    
    // Clear window buffer
    for (int i = 0; i < width * height; i++) {
        win->buffer[i] = GFX_COLOR_LIGHT_GRAY;
    }
    
    return wm.window_count++;
}

void wm_destroy_window(int window_id) {
    if (window_id < 0 || window_id >= wm.window_count) return;
    
    if (wm.windows[window_id].buffer) {
        pmm_free_block(wm.windows[window_id].buffer);
    }
    
    for (int i = window_id; i < wm.window_count - 1; i++) {
        wm.windows[i] = wm.windows[i + 1];
    }
    
    wm.window_count--;
    
    // Если уничтожили активное окно, сбрасываем активное окно
    if (wm.active_window == window_id) {
        wm.active_window = -1;
    } else if (wm.active_window > window_id) {
        wm.active_window--;
    }
}

void wm_draw_window(int window_id) {
    if (window_id < 0 || window_id >= wm.window_count) return;
    
    Window* win = &wm.windows[window_id];
    if (!win->visible) return;
    
    // Draw window shadow (Windows 2.0 style)
    gfx_fill_rect(win->x + 4, win->y + 4, win->width, win->height, GFX_COLOR_DARK_GRAY);
    
    // Draw main window background
    gfx_fill_rect(win->x, win->y, win->width, win->height, GFX_COLOR_WHITE);
    
    // Draw window border
    gfx_draw_rect(win->x, win->y, win->width, win->height, GFX_COLOR_BLACK);
    
    // Draw title bar (blue for active, gray for inactive)
    uint8_t title_color = (window_id == wm.active_window) ? GFX_COLOR_BLUE : GFX_COLOR_DARK_GRAY;
    gfx_fill_rect(win->x + 2, win->y + 2, win->width - 4, WINDOW_TITLE_HEIGHT, title_color);
    
    // Draw title text
    gfx_draw_string(win->x + 8, win->y + 8, win->title, GFX_COLOR_WHITE);
    
    // Draw close button (Windows 2.0 style - simple X)
    gfx_draw_rect(win->x + win->width - 20, win->y + 6, 14, 12, GFX_COLOR_WHITE);
    gfx_draw_string(win->x + win->width - 16, win->y + 8, "x", GFX_COLOR_BLACK);
    
    // Draw window content area
    gfx_draw_rect(win->x + 2, win->y + WINDOW_TITLE_HEIGHT + 2, 
                 win->width - 4, win->height - WINDOW_TITLE_HEIGHT - 4, 
                 GFX_COLOR_BLACK);
    
    // Draw content background
    gfx_fill_rect(win->x + 3, win->y + WINDOW_TITLE_HEIGHT + 3, 
                 win->width - 6, win->height - WINDOW_TITLE_HEIGHT - 6, 
                 GFX_COLOR_LIGHT_GRAY);
    
    // Copy window buffer to screen
    for (int y = 0; y < win->height - WINDOW_TITLE_HEIGHT - 8; y++) {
        for (int x = 0; x < win->width - 6; x++) {
            if (win->x + x + 3 < wm.screen_width && 
                win->y + y + WINDOW_TITLE_HEIGHT + 3 < wm.screen_height) {
                gfx_draw_pixel(win->x + x + 3, 
                              win->y + y + WINDOW_TITLE_HEIGHT + 3, 
                              win->buffer[y * win->width + x]);
            }
        }
    }
    
    win->needs_redraw = 0;
}

void wm_redraw_all() {
    // Draw desktop background (Windows 2.0 light blue)
    gfx_clear_screen(GFX_COLOR_LIGHT_BLUE);
    
    // Draw all windows from bottom to top
    for (int i = 0; i < wm.window_count; i++) {
        if (i != wm.active_window) {
            wm_draw_window(i);
        }
    }
    
    // Draw active window last (on top)
    if (wm.active_window >= 0) {
        wm_draw_window(wm.active_window);
    }
    
    gfx_update();
}

void wm_set_active(int window_id) {
    if (window_id >= 0 && window_id < wm.window_count) {
        wm.active_window = window_id;
        for (int i = 0; i < wm.window_count; i++) {
            wm.windows[i].active = (i == window_id);
            wm.windows[i].needs_redraw = 1;
        }
    }
}

void wm_move_window(int window_id, int x, int y) {
    if (window_id >= 0 && window_id < wm.window_count) {
        wm.windows[window_id].x = x;
        wm.windows[window_id].y = y;
        wm.windows[window_id].needs_redraw = 1;
    }
}

void wm_handle_click(int x, int y) {
    // Check windows from top to bottom
    for (int i = wm.window_count - 1; i >= 0; i--) {
        Window* win = &wm.windows[i];
        if (win->visible && 
            x >= win->x && x <= win->x + win->width &&
            y >= win->y && y <= win->y + win->height) {
            
            // Check if close button clicked
            if (x >= win->x + win->width - 20 && x <= win->x + win->width - 6 &&
                y >= win->y + 6 && y <= win->y + 18) {
                wm_destroy_window(i);
                wm_redraw_all();
                return;
            }
            
            wm_set_active(i);
            
            // Simple window dragging (title bar)
            if (y >= win->y && y <= win->y + WINDOW_TITLE_HEIGHT) {
                // Basic drag implementation
                int new_x = x - win->width/2;
                int new_y = y - WINDOW_TITLE_HEIGHT/2;
                if (new_x < 0) new_x = 0;
                if (new_y < 0) new_y = 0;
                if (new_x > wm.screen_width - win->width) new_x = wm.screen_width - win->width;
                if (new_y > wm.screen_height - win->height) new_y = wm.screen_height - win->height;
                wm_move_window(i, new_x, new_y);
            }
            
            wm_redraw_all();
            return;
        }
    }
}

// Terminal window implementation
static int terminal_window_id = -1;
static int term_cursor_x = 5;
static int term_cursor_y = 5;

void wm_create_terminal_window() {
    terminal_window_id = wm_create_window(50, 50, 400, 300, "ViX Terminal");
    if (terminal_window_id != -1) {
        wm_set_active(terminal_window_id);
        wm_terminal_writestring("ViXOS Terminal v0.3\n");
        wm_terminal_writestring("Type 'help' for commands\n");
        wm_terminal_writestring("Press ESC to exit GUI\n");
        wm_terminal_writestring("> ");
    }
}

void wm_terminal_clear() {
    if (terminal_window_id == -1) return;
    
    Window* win = &wm.windows[terminal_window_id];
    for (int i = 0; i < win->width * win->height; i++) {
        win->buffer[i] = GFX_COLOR_LIGHT_GRAY;
    }
    term_cursor_x = 5;
    term_cursor_y = 5;
    win->needs_redraw = 1;
}

void wm_terminal_putchar(char c) {
    if (terminal_window_id == -1) return;
    
    Window* win = &wm.windows[terminal_window_id];
    int content_width = win->width - 10;
    int content_height = win->height - WINDOW_TITLE_HEIGHT - 10;
    
    if (c == '\n') {
        term_cursor_x = 5;
        term_cursor_y += 8;
        if (term_cursor_y > content_height - 8) {
            // Simple scroll - just clear and reset
            wm_terminal_clear();
            term_cursor_y = 5;
            wm_terminal_writestring("> ");
        }
        return;
    }
    
    if (c == '\b') { // Backspace
        if (term_cursor_x > 5) {
            term_cursor_x -= 5;
            // Clear the character
            for (int dy = 0; dy < 6; dy++) {
                for (int dx = 0; dx < 4; dx++) {
                    int pixel_x = term_cursor_x + dx;
                    int pixel_y = term_cursor_y + dy;
                    if (pixel_x < win->width && pixel_y < win->height) {
                        win->buffer[pixel_y * win->width + pixel_x] = GFX_COLOR_LIGHT_GRAY;
                    }
                }
            }
        }
        win->needs_redraw = 1;
        return;
    }
    
    // Draw character to window buffer
    int buf_x = term_cursor_x;
    int buf_y = term_cursor_y;
    
    if (buf_x < content_width && buf_y < content_height) {
        // Simple character rendering (4x6 pixels)
        for (int dy = 0; dy < 6; dy++) {
            for (int dx = 0; dx < 4; dx++) {
                int pixel_x = buf_x + dx;
                int pixel_y = buf_y + dy;
                if (pixel_x < win->width && pixel_y < win->height) {
                    win->buffer[pixel_y * win->width + pixel_x] = GFX_COLOR_BLACK;
                }
            }
        }
    }
    
    term_cursor_x += 5;
    
    if (term_cursor_x > content_width - 5) {
        term_cursor_x = 5;
        term_cursor_y += 8;
    }
    
    win->needs_redraw = 1;
}

void wm_terminal_writestring(const char* str) {
    while (*str) {
        wm_terminal_putchar(*str++);
    }
}