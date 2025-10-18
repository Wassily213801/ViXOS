#include "gfx.h"
#include "video.h"
#include "string.h"
#include "port_io.h"

// Define the global graphics context
GFX_Context gfx_ctx;

// Простая замена printf для вывода отладочной информации
static void debug_print(const char* str) {
    // Используем существующую функцию вывода в видео-буфер
    video_print(str);
}

// Simple 8x8 font (only first 128 ASCII characters)
static const uint8_t font8x8[128][8] = {
    // Basic font data would go here
    // This is just a placeholder
    [0 ... 127] = {0} // Initialize all to 0 for now
};

void gfx_init() {
    video_init_graphics();
    video_switch_to_graphics();
    
    gfx_ctx.width = video_get_width();
    gfx_ctx.height = video_get_height();
    gfx_ctx.framebuffer = video_get_framebuffer();
    
    // Простой вывод отладочной информации без printf
    debug_print("Graphics initialized\n");
    debug_print("Resolution: ");
    // Здесь нужно преобразовать числа в строку - простая реализация:
    char width_str[10] = {0};
    char height_str[10] = {0};
    itoa(gfx_ctx.width, width_str, 10);
    itoa(gfx_ctx.height, height_str, 10);
    debug_print(width_str);
    debug_print("x");
    debug_print(height_str);
    debug_print("\n");
    
    // Set up a basic palette
    for (int i = 0; i < 16; i++) {
        gfx_set_palette(i, i*16, i*16, i*16);
    }
}

// ... остальные функции остаются без изменений ...
void gfx_clear_screen(uint8_t color) {
    memset(gfx_ctx.framebuffer, color, gfx_ctx.width * gfx_ctx.height);
}

void gfx_draw_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < gfx_ctx.width && y >= 0 && y < gfx_ctx.height) {
        gfx_ctx.framebuffer[y * gfx_ctx.width + x] = color;
    }
}


void gfx_draw_rect(int x, int y, int width, int height, uint8_t color) {
    video_draw_rect(x, y, width, height, color);
}

void gfx_fill_rect(int x, int y, int width, int height, uint8_t color) {
    video_fill_rect(x, y, width, height, color);
}

void gfx_draw_line(int x1, int y1, int x2, int y2, uint8_t color) {
    video_draw_line(x1, y1, x2, y2, color);
}

void gfx_draw_char(int x, int y, char c, uint8_t color) {
    if (c < 0 || c >= 128) return;
    
    for (int dy = 0; dy < 8; dy++) {
        for (int dx = 0; dx < 8; dx++) {
            if (font8x8[(int)c][dy] & (1 << dx)) {
                gfx_draw_pixel(x + dx, y + dy, color);
            }
        }
    }
}

void gfx_draw_string(int x, int y, const char* str, uint8_t color) {
    while (*str) {
        gfx_draw_char(x, y, *str++, color);
        x += 8;
        if (x >= gfx_ctx.width - 8) {
            x = 0;
            y += 8;
        }
    }
}

void gfx_update() {
    // For now, we're using direct framebuffer access
    // In a real implementation, this might do double buffering
}

void gfx_set_palette(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    // Set VGA palette (placeholder)
    // Actual implementation depends on your hardware
    outb(0x3C8, index);
    outb(0x3C9, r >> 2);
    outb(0x3C9, g >> 2);
    outb(0x3C9, b >> 2);
}