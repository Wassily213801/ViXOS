#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Graphics modes
typedef enum {
    GFX_MODE_640x480_8BIT,
    GFX_MODE_800x600_8BIT,
    GFX_MODE_1024x768_8BIT
} GraphicsMode;

enum VGAColor {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_YELLOW = 14,
    COLOR_WHITE = 15,
    COLOR_DARK_BLUE = 16,
};

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} TextAlignHorizontal;

typedef enum {
    ALIGN_TOP,
    ALIGN_MIDDLE,
    ALIGN_BOTTOM
} TextAlignVertical;

// Basic text mode functions
void video_clear(void);
void video_putc(char c);
void video_print(const char* str);
void video_print_at(const char* str, TextAlignHorizontal halign, TextAlignVertical valign);
void video_backspace(void);
void video_set_color(uint8_t fg, uint8_t bg);
void video_set_cursor(int x, int y);
void video_print_int(int num);

// Graphics mode functions
void video_init_graphics(void);
void video_set_graphics_mode(int mode);
void video_set_pixel(int x, int y, uint8_t color);
uint8_t video_get_pixel(int x, int y);
void video_draw_rect(int x, int y, int width, int height, uint8_t color);
void video_fill_rect(int x, int y, int width, int height, uint8_t color);
void video_draw_line(int x1, int y1, int x2, int y2, uint8_t color);
void video_draw_circle(int center_x, int center_y, int radius, uint8_t color);
void video_fill_circle(int center_x, int center_y, int radius, uint8_t color);
void video_switch_to_text(void);
void video_switch_to_graphics(void);
uint8_t* video_get_framebuffer(void);
int video_get_width(void);
int video_get_height(void);
int video_get_graphics_mode(void);

// Double buffering functions
void video_enable_double_buffering(int enable);
void video_swap_buffers(void);
void video_clear_back_buffer(uint8_t color);

// Font and text in graphics mode
void video_draw_char(int x, int y, char c, uint8_t color);
void video_draw_string(int x, int y, const char* str, uint8_t color);

// Enhanced window management functions
int video_create_window(int x, int y, int width, int height, const char* title);
void video_destroy_window(int win_id);
void video_draw_window(int win_id);
void video_set_window_pixel(int win_id, int x, int y, uint8_t color);
void video_fill_window(int win_id, uint8_t color);
void video_move_window(int win_id, int x, int y);
void video_resize_window(int win_id, int width, int height);
void video_show_window(int win_id, int visible);
void video_set_window_title(int win_id, const char* title);
void video_set_window_border(int win_id, int has_border, uint8_t border_color);

// Enhanced animation functions
int video_create_animation(int x, int y, int frame_count, int speed, int loop);
void video_destroy_animation(int anim_id);
void video_set_animation_frame(int anim_id, int frame, const uint8_t* data);
void video_draw_animation_frame(int anim_id, int frame);
void video_play_animation(int anim_id);
void video_update_animations(void);
void video_stop_animation(int anim_id);
void video_set_animation_position(int anim_id, int x, int y);

#endif
