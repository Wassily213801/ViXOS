#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Увеличенные разрешения
typedef enum {
    GFX_MODE_640x480_8BIT,
    GFX_MODE_800x600_8BIT,
    GFX_MODE_1024x768_8BIT,
    GFX_MODE_1280x1024_8BIT
} GraphicsMode;

// Расширенная палитра цветов (256 цветов)
enum VGAColor {
    // Основные 16 цветов VGA
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
    
    // Дополнительные цвета
    COLOR_DARK_BLUE = 16,
    COLOR_PURPLE = 17,
    COLOR_ORANGE = 18,
    COLOR_PINK = 19,
    COLOR_TEAL = 20,
    COLOR_MAROON = 21,
    COLOR_OLIVE = 22,
    COLOR_NAVY = 23,
    COLOR_GRAY = 24,
    COLOR_SILVER = 25,
    
    // Градиенты
    COLOR_GRADIENT_START = 32,
    COLOR_GRADIENT_END = 255
};

// GUI стили
typedef enum {
    WINDOW_STYLE_DEFAULT,
    WINDOW_STYLE_MODERN,
    WINDOW_STYLE_CLASSIC,
    WINDOW_STYLE_MINIMAL
} WindowStyle;

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

// Структура для кнопки
typedef struct {
    int x, y;
    int width, height;
    char label[32];
    uint8_t bg_color;
    uint8_t text_color;
    uint8_t hover_color;
    uint8_t active_color;
    int is_hovered;
    int is_pressed;
    void (*on_click)(void);
} GUIButton;

// Структура для панели
typedef struct {
    int x, y;
    int width, height;
    uint8_t bg_color;
    char title[64];
    GUIButton* buttons;
    int button_count;
} GUIPanel;

// Basic text mode functions
void video_clear(void);
void video_clear_with_color(uint8_t fg, uint8_t bg);
void video_putc(char c);
void video_print(const char* str);
void video_print_with_color(const char* str, uint8_t fg, uint8_t bg);
void video_print_at(const char* str, TextAlignHorizontal halign, TextAlignVertical valign);
void video_print_at_with_color(const char* str, TextAlignHorizontal halign, 
                               TextAlignVertical valign, uint8_t fg, uint8_t bg);
void video_backspace(void);
void video_set_color(uint8_t fg, uint8_t bg);
void video_set_text_color(uint8_t fg);
void video_set_bg_color(uint8_t bg);
uint8_t video_get_text_color(void);
uint8_t video_get_bg_color(void);
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
void video_draw_rounded_rect(int x, int y, int width, int height, int radius, uint8_t color);
void video_fill_rounded_rect(int x, int y, int width, int height, int radius, uint8_t color);
void video_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint8_t color);
void video_fill_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint8_t color);
void video_draw_gradient(int x, int y, int width, int height, uint8_t start_color, uint8_t end_color, int vertical);
void video_drop_shadow(int x, int y, int width, int height, int radius, uint8_t shadow_color);

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
void video_draw_string_shadow(int x, int y, const char* str, uint8_t color, uint8_t shadow_color);

// GUI элементы
void video_draw_button(int x, int y, int width, int height, const char* text, 
                       uint8_t bg_color, uint8_t text_color, int is_pressed);
void video_draw_panel(int x, int y, int width, int height, const char* title, 
                      uint8_t bg_color, uint8_t border_color, int has_shadow);
void video_draw_progress_bar(int x, int y, int width, int height, int progress, 
                             uint8_t bg_color, uint8_t fill_color, uint8_t border_color);
void video_draw_slider(int x, int y, int width, int value, 
                       uint8_t track_color, uint8_t slider_color, uint8_t border_color);
void video_draw_checkbox(int x, int y, int size, int checked, 
                         uint8_t bg_color, uint8_t check_color, uint8_t border_color);
void video_draw_textbox(int x, int y, int width, int height, const char* text, 
                        uint8_t bg_color, uint8_t text_color, uint8_t border_color, int has_focus);
void video_draw_scrollbar(int x, int y, int width, int height, int thumb_position, 
                          uint8_t bg_color, uint8_t thumb_color, uint8_t border_color);

// Системный GUI (как на скриншоте)
void video_draw_system_gui(void);
void video_draw_terminal_window(void);
void video_draw_system_info_panel(void);
void video_draw_menu_bar(void);
void video_draw_status_bar(void);

// Enhanced window management functions
int video_create_window(int x, int y, int width, int height, const char* title, WindowStyle style);
void video_destroy_window(int win_id);
void video_draw_window(int win_id);
void video_set_window_pixel(int win_id, int x, int y, uint8_t color);
void video_fill_window(int win_id, uint8_t color);
void video_move_window(int win_id, int x, int y);
void video_resize_window(int win_id, int width, int height);
void video_show_window(int win_id, int visible);
void video_set_window_title(int win_id, const char* title);
void video_set_window_border(int win_id, int has_border, uint8_t border_color);
void video_set_window_style(int win_id, WindowStyle style);
void video_draw_window_controls(int win_id);

// Enhanced animation functions
int video_create_animation(int x, int y, int frame_count, int speed, int loop);
void video_destroy_animation(int anim_id);
void video_set_animation_frame(int anim_id, int frame, const uint8_t* data);
void video_draw_animation_frame(int anim_id, int frame);
void video_play_animation(int anim_id);
void video_update_animations(void);
void video_stop_animation(int anim_id);
void video_set_animation_position(int anim_id, int x, int y);

// Утилиты цвета
uint8_t video_color_rgb(uint8_t r, uint8_t g, uint8_t b);
uint8_t video_color_hsv(int hue, int saturation, int value);
uint8_t video_color_blend(uint8_t color1, uint8_t color2, uint8_t alpha);
void video_set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

#endif