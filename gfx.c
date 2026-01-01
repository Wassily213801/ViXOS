// gfx.c (улучшенная версия с полным шрифтом)
#include "gfx.h"
#include "video.h"
#include "string.h"
#include "font8x8.h"

GFX_Context gfx_ctx;

void gfx_init() {
    gfx_ctx.width = video_get_width();
    gfx_ctx.height = video_get_height();
    gfx_ctx.framebuffer = video_get_framebuffer();
}

void gfx_clear_screen(uint8_t color) {
    for (int i = 0; i < gfx_ctx.width * gfx_ctx.height; i++) {
        gfx_ctx.framebuffer[i] = color;
    }
}

void gfx_draw_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < gfx_ctx.width && y >= 0 && y < gfx_ctx.height) {
        gfx_ctx.framebuffer[y * gfx_ctx.width + x] = color;
    }
}

void gfx_draw_rect(int x, int y, int width, int height, uint8_t color) {
    // Top and bottom
    for (int i = x; i < x + width; i++) {
        gfx_draw_pixel(i, y, color);
        gfx_draw_pixel(i, y + height - 1, color);
    }
    // Left and right
    for (int j = y; j < y + height; j++) {
        gfx_draw_pixel(x, j, color);
        gfx_draw_pixel(x + width - 1, j, color);
    }
}

void gfx_fill_rect(int x, int y, int width, int height, uint8_t color) {
    for (int i = x; i < x + width; i++) {
        for (int j = y; j < y + height; j++) {
            gfx_draw_pixel(i, j, color);
        }
    }
}

void gfx_draw_line(int x1, int y1, int x2, int y2, uint8_t color) {
    int dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        gfx_draw_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void gfx_draw_char(int x, int y, char c, uint8_t color) {
    unsigned char uc = (unsigned char)c;
    if (uc >= 128) return;
    const uint8_t* glyph = font8x8_basic[uc];
    for (int row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << col)) {
                gfx_draw_pixel(x + col, y + row, color);
            }
        }
    }
}

void gfx_draw_string(int x, int y, const char* str, uint8_t color) {
    int start_x = x;
    while (*str) {
        if (*str == '\n') {
            y += 10;
            x = start_x;
            str++;
            continue;
        }
        gfx_draw_char(x, y, *str, color);
        x += 9; // 8px glyph + 1px spacing
        if (x >= gfx_ctx.width - 8) {
            x = start_x;
            y += 10;
        }
        str++;
    }
}

void gfx_update() {
    // Обновляем экран: копируем back-buffer в front-buffer.
    // В текущей реализации реальная аппаратная видеопамять не трогается,
    // но это позволит увидеть изменения в переднем буфере при наличии
    // соответствующей реализации вывода.
    video_swap_buffers();
    gfx_ctx.framebuffer = video_get_framebuffer();
}