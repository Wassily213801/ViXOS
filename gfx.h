#ifndef GFX_H
#define GFX_H

#include <stdint.h>

// Color definitions
#define GFX_COLOR_BLACK     0x00
#define GFX_COLOR_WHITE     0x0F
#define GFX_COLOR_GRAY      0x07
#define GFX_COLOR_DARK_GRAY 0x08
#define GFX_COLOR_BLUE      0x01
#define GFX_COLOR_RED       0x04
#define GFX_COLOR_GREEN     0x02

// Graphics context structure
typedef struct {
    int width;
    int height;
    uint8_t* framebuffer;
} GFX_Context;

// Global graphics context
extern GFX_Context gfx_ctx;

// Initialization
void gfx_init();
void gfx_clear_screen(uint8_t color);

// Drawing functions
void gfx_draw_pixel(int x, int y, uint8_t color);
void gfx_draw_rect(int x, int y, int width, int height, uint8_t color);
void gfx_fill_rect(int x, int y, int width, int height, uint8_t color);
void gfx_draw_line(int x1, int y1, int x2, int y2, uint8_t color);

// Text rendering
void gfx_draw_char(int x, int y, char c, uint8_t color);
void gfx_draw_string(int x, int y, const char* str, uint8_t color);

// Screen management
void gfx_update();
void gfx_set_palette(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

#endif