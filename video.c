#include "video.h"
#include "port_io.h"
#include "string.h"

// Конфигурация с увеличенными разрешениями
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define TEXT_MEMORY ((volatile uint16_t*)0xB8000)

// Выбор разрешения по умолчанию (1024x768)
#define GFX_WIDTH 1024
#define GFX_HEIGHT 768
#define GFX_BPP 8
#define GFX_MEMORY ((volatile uint8_t*)0xE0000000)
#define GFX_BUFFER_SIZE (GFX_WIDTH * GFX_HEIGHT)

// GUI конфигурация
#define MAX_WINDOWS 20
#define MAX_WINDOW_WIDTH 800
#define MAX_WINDOW_HEIGHT 600
#define MAX_BUTTONS 50
#define MAX_ANIMATIONS 20
#define MAX_ANIMATION_FRAMES 30
#define ANIMATION_FRAME_SIZE (64*64)

// Шрифты
#define FONT_WIDTH 8
#define FONT_HEIGHT 16
#define FONT_CHARS 256
#define FONT_SMALL_WIDTH 6
#define FONT_SMALL_HEIGHT 8

// Цветовая палитра (256 цветов)
static uint8_t color_palette[256][3];

// Текущие настройки
static uint8_t text_color = 0x07;
static uint8_t bg_color = 0x00;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static int graphics_mode = 0;
static int current_gfx_mode = GFX_MODE_1024x768_8BIT;

// Двойная буферизация
static uint8_t front_buffer[GFX_BUFFER_SIZE];
static uint8_t back_buffer[GFX_BUFFER_SIZE];
static int double_buffering_enabled = 1;

// Управление окнами
typedef struct {
    int x, y;
    int width, height;
    char title[64];
    uint8_t buffer[MAX_WINDOW_WIDTH * MAX_WINDOW_HEIGHT];
    uint8_t visible;
    uint8_t has_border;
    uint8_t border_color;
    uint8_t background_color;
    uint8_t title_bar_color;
    WindowStyle style;
    int is_minimized;
    int is_maximized;
    int z_index;
} GfxWindow;

static GfxWindow windows[MAX_WINDOWS];
static int window_count = 0;

// GUI элементы
static GUIButton buttons[MAX_BUTTONS];
static int button_count = 0;

// Анимации
typedef struct {
    int x, y;
    int frame_count;
    int current_frame;
    uint8_t frames[MAX_ANIMATION_FRAMES][ANIMATION_FRAME_SIZE];
    int active;
    int loop;
    int speed;
    int frame_delay;
    int frame_counter;
    int width;
    int height;
} Animation;

static Animation animations[MAX_ANIMATIONS];
static int animation_count = 0;

// Данные шрифта (упрощенные)
static const uint8_t default_font[FONT_CHARS][FONT_HEIGHT] = {0};
static const uint8_t small_font[FONT_CHARS][FONT_SMALL_HEIGHT] = {0};

// ========== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ==========

static int absolute_value(int n) {
    return n < 0 ? -n : n;
}

static void int_to_string(int num, char* buffer, int base) {
    char* p = buffer;
    char* p1, *p2;
    int digits = 0;
    
    if (num == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }
    
    if (num < 0 && base == 10) {
        *p++ = '-';
        num = -num;
    }
    
    p1 = p;
    while (num != 0) {
        int remainder = num % base;
        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
        num = num / base;
    }
    *p-- = '\0';
    
    p2 = p1;
    while (p2 < p) {
        char tmp = *p2;
        *p2++ = *p;
        *p-- = tmp;
    }
}

static uint8_t make_vga_attribute(uint8_t fg, uint8_t bg) {
    return (bg << 4) | (fg & 0x0F);
}

// Быстрая аппроксимация sqrt для целых чисел
static int fast_sqrt(int x) {
    if (x <= 1) return x;
    
    int start = 1, end = x, ans = 0;
    while (start <= end) {
        int mid = (start + end) / 2;
        if (mid * mid == x) return mid;
        if (mid * mid < x) {
            start = mid + 1;
            ans = mid;
        } else {
            end = mid - 1;
        }
    }
    return ans;
}

// ========== ИНИЦИАЛИЗАЦИЯ ЦВЕТОВОЙ ПАЛИТРЫ ==========

void video_init_color_palette(void) {
    // VGA стандартные 16 цветов
    for (int i = 0; i < 16; i++) {
        switch (i) {
            case COLOR_BLACK: color_palette[i][0]=0; color_palette[i][1]=0; color_palette[i][2]=0; break;
            case COLOR_BLUE: color_palette[i][0]=0; color_palette[i][1]=0; color_palette[i][2]=170; break;
            case COLOR_GREEN: color_palette[i][0]=0; color_palette[i][1]=170; color_palette[i][2]=0; break;
            case COLOR_CYAN: color_palette[i][0]=0; color_palette[i][1]=170; color_palette[i][2]=170; break;
            case COLOR_RED: color_palette[i][0]=170; color_palette[i][1]=0; color_palette[i][2]=0; break;
            case COLOR_MAGENTA: color_palette[i][0]=170; color_palette[i][1]=0; color_palette[i][2]=170; break;
            case COLOR_BROWN: color_palette[i][0]=170; color_palette[i][1]=85; color_palette[i][2]=0; break;
            case COLOR_LIGHT_GREY: color_palette[i][0]=170; color_palette[i][1]=170; color_palette[i][2]=170; break;
            case COLOR_DARK_GREY: color_palette[i][0]=85; color_palette[i][1]=85; color_palette[i][2]=85; break;
            case COLOR_LIGHT_BLUE: color_palette[i][0]=85; color_palette[i][1]=85; color_palette[i][2]=255; break;
            case COLOR_LIGHT_GREEN: color_palette[i][0]=85; color_palette[i][1]=255; color_palette[i][2]=85; break;
            case COLOR_LIGHT_CYAN: color_palette[i][0]=85; color_palette[i][1]=255; color_palette[i][2]=255; break;
            case COLOR_LIGHT_RED: color_palette[i][0]=255; color_palette[i][1]=85; color_palette[i][2]=85; break;
            case COLOR_LIGHT_MAGENTA: color_palette[i][0]=255; color_palette[i][1]=85; color_palette[i][2]=255; break;
            case COLOR_YELLOW: color_palette[i][0]=255; color_palette[i][1]=255; color_palette[i][2]=85; break;
            case COLOR_WHITE: color_palette[i][0]=255; color_palette[i][1]=255; color_palette[i][2]=255; break;
        }
    }
    
    // Градиенты и дополнительные цвета
    for (int i = 32; i < 256; i++) {
        int hue = (i - 32) * 360 / 224;
        int sat = 200;
        int val = 200;
        
        // HSV to RGB conversion
        int h = hue / 60;
        int f = (hue % 60) * 255 / 60;
        int p = (val * (255 - sat)) / 255;
        int q = (val * (255 - f * sat / 255)) / 255;
        int t = (val * (255 - (255 - f) * sat / 255)) / 255;
        
        switch (h) {
            case 0: color_palette[i][0] = val; color_palette[i][1] = t; color_palette[i][2] = p; break;
            case 1: color_palette[i][0] = q; color_palette[i][1] = val; color_palette[i][2] = p; break;
            case 2: color_palette[i][0] = p; color_palette[i][1] = val; color_palette[i][2] = t; break;
            case 3: color_palette[i][0] = p; color_palette[i][1] = q; color_palette[i][2] = val; break;
            case 4: color_palette[i][0] = t; color_palette[i][1] = p; color_palette[i][2] = val; break;
            case 5: color_palette[i][0] = val; color_palette[i][1] = p; color_palette[i][2] = q; break;
        }
    }
}

// ========== ТЕКСТОВЫЙ РЕЖИМ ==========

static void scroll_screen_if_needed(void) {
    if (cursor_y < VGA_HEIGHT) return;

    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            TEXT_MEMORY[(y - 1) * VGA_WIDTH + x] = TEXT_MEMORY[y * VGA_WIDTH + x];
        }
    }

    uint8_t current_attribute = make_vga_attribute(text_color, bg_color);
    for (int x = 0; x < VGA_WIDTH; x++) {
        TEXT_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = ((uint16_t)current_attribute << 8) | ' ';
    }

    cursor_y = VGA_HEIGHT - 1;
}

static void update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x0F, 0x3D4);
    outb((uint8_t)(pos & 0xFF), 0x3D5);
    outb(0x0E, 0x3D4);
    outb((uint8_t)((pos >> 8) & 0xFF), 0x3D5);
}

void video_clear(void) {
    uint8_t current_attribute = make_vga_attribute(text_color, bg_color);
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            TEXT_MEMORY[y * VGA_WIDTH + x] = ((uint16_t)current_attribute << 8) | ' ';
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void video_clear_with_color(uint8_t fg, uint8_t bg) {
    uint8_t attribute = make_vga_attribute(fg, bg);
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            TEXT_MEMORY[y * VGA_WIDTH + x] = ((uint16_t)attribute << 8) | ' ';
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    text_color = fg;
    bg_color = bg;
    update_cursor();
}

void video_putc(char c) {
    uint8_t current_attribute = make_vga_attribute(text_color, bg_color);
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) cursor_x--;
        TEXT_MEMORY[cursor_y * VGA_WIDTH + cursor_x] = ((uint16_t)current_attribute << 8) | ' ';
    } else {
        TEXT_MEMORY[cursor_y * VGA_WIDTH + cursor_x] = ((uint16_t)current_attribute << 8) | (uint8_t)c;
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    scroll_screen_if_needed();
    update_cursor();
}

void video_print(const char* str) {
    while (*str) {
        video_putc(*str++);
    }
}

void video_print_with_color(const char* str, uint8_t fg, uint8_t bg) {
    uint8_t saved_text_color = text_color;
    uint8_t saved_bg_color = bg_color;
    
    text_color = fg;
    bg_color = bg;
    
    video_print(str);
    
    text_color = saved_text_color;
    bg_color = saved_bg_color;
}

void video_print_at(const char* str, TextAlignHorizontal halign, TextAlignVertical valign) {
    int orig_x = cursor_x;
    int orig_y = cursor_y;
    
    switch (valign) {
        case ALIGN_TOP: cursor_y = 0; break;
        case ALIGN_MIDDLE: cursor_y = (VGA_HEIGHT / 2) - 1; break;
        case ALIGN_BOTTOM: cursor_y = VGA_HEIGHT - 1; break;
    }
    
    int len = 0;
    const char* p = str;
    while (*p++) len++;
    
    switch (halign) {
        case ALIGN_LEFT: cursor_x = 0; break;
        case ALIGN_CENTER: cursor_x = (VGA_WIDTH - len) / 2; break;
        case ALIGN_RIGHT: cursor_x = VGA_WIDTH - len; break;
    }
    
    if (cursor_x < 0) cursor_x = 0;
    video_print(str);
    
    cursor_x = orig_x;
    cursor_y = orig_y;
}

void video_print_at_with_color(const char* str, TextAlignHorizontal halign, 
                               TextAlignVertical valign, uint8_t fg, uint8_t bg) {
    uint8_t saved_text_color = text_color;
    uint8_t saved_bg_color = bg_color;
    
    text_color = fg;
    bg_color = bg;
    
    video_print_at(str, halign, valign);
    
    text_color = saved_text_color;
    bg_color = saved_bg_color;
}

// ДОБАВЛЕНО: Функция video_backspace которая отсутствовала
void video_backspace(void) {
    if (cursor_x > 0) {
        cursor_x--;
    } else if (cursor_y > 0) {
        cursor_y--;
        cursor_x = VGA_WIDTH - 1;
    }
    
    uint8_t current_attribute = make_vga_attribute(text_color, bg_color);
    TEXT_MEMORY[cursor_y * VGA_WIDTH + cursor_x] = ((uint16_t)current_attribute << 8) | ' ';
    update_cursor();
}

void video_set_cursor(int x, int y) {
    if (x < 0) x = 0;
    if (x >= VGA_WIDTH) x = VGA_WIDTH - 1;
    if (y < 0) y = 0;
    if (y >= VGA_HEIGHT) y = VGA_HEIGHT - 1;
    
    cursor_x = x;
    cursor_y = y;
    update_cursor();
}

void video_set_color(uint8_t fg, uint8_t bg) {
    text_color = fg & 0x0F;
    bg_color = bg & 0x0F;
}

void video_set_text_color(uint8_t fg) {
    text_color = fg & 0x0F;
}

void video_set_bg_color(uint8_t bg) {
    bg_color = bg & 0x0F;
}

uint8_t video_get_text_color(void) {
    return text_color;
}

uint8_t video_get_bg_color(void) {
    return bg_color;
}

// ========== ГРАФИЧЕСКИЙ РЕЖИМ ==========

void video_init_graphics(void) {
    graphics_mode = 1;
    current_gfx_mode = GFX_MODE_1024x768_8BIT;
    video_init_color_palette();
    
    for (int i = 0; i < GFX_BUFFER_SIZE; i++) {
        GFX_MEMORY[i] = COLOR_BLACK;
        if (double_buffering_enabled) {
            front_buffer[i] = COLOR_BLACK;
            back_buffer[i] = COLOR_BLACK;
        }
    }
}

void video_set_graphics_mode(int mode) {
    current_gfx_mode = mode;
    // Здесь должна быть реальная инициализация видеорежима через VBE
}

void video_set_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < GFX_WIDTH && y >= 0 && y < GFX_HEIGHT) {
        if (double_buffering_enabled) {
            back_buffer[y * GFX_WIDTH + x] = color;
        } else {
            GFX_MEMORY[y * GFX_WIDTH + x] = color;
        }
    }
}

uint8_t video_get_pixel(int x, int y) {
    if (x >= 0 && x < GFX_WIDTH && y >= 0 && y < GFX_HEIGHT) {
        if (double_buffering_enabled) {
            return back_buffer[y * GFX_WIDTH + x];
        } else {
            return GFX_MEMORY[y * GFX_WIDTH + x];
        }
    }
    return 0;
}

// Улучшенные графические примитивы
void video_draw_rect(int x, int y, int width, int height, uint8_t color) {
    for (int i = x; i < x + width; i++) {
        video_set_pixel(i, y, color);
        video_set_pixel(i, y + height - 1, color);
    }
    for (int j = y; j < y + height; j++) {
        video_set_pixel(x, j, color);
        video_set_pixel(x + width - 1, j, color);
    }
}

void video_fill_rect(int x, int y, int width, int height, uint8_t color) {
    for (int i = x; i < x + width && i < GFX_WIDTH; i++) {
        for (int j = y; j < y + height && j < GFX_HEIGHT; j++) {
            if (i >= 0 && j >= 0) {
                video_set_pixel(i, j, color);
            }
        }
    }
}

// Оптимизированная версия без sqrt
void video_draw_rounded_rect(int x, int y, int width, int height, int radius, uint8_t color) {
    // Рисуем основные прямые стороны
    video_draw_rect(x + radius, y, width - 2 * radius, height, color);
    video_draw_rect(x, y + radius, width, height - 2 * radius, color);
    
    // Рисуем углы с помощью простого алгоритма (без sqrt)
    int r2 = radius * radius;
    for (int i = 0; i <= radius; i++) {
        // Вычисляем j используя таблицу квадратов
        int j = 0;
        while (j * j + i * i <= r2 && j <= radius) {
            j++;
        }
        j--;
        
        // Верхний левый угол
        video_set_pixel(x + radius - i, y + radius - j, color);
        video_set_pixel(x + radius - j, y + radius - i, color);
        
        // Верхний правый угол
        video_set_pixel(x + width - radius + i - 1, y + radius - j, color);
        video_set_pixel(x + width - radius + j - 1, y + radius - i, color);
        
        // Нижний левый угол
        video_set_pixel(x + radius - i, y + height - radius + j - 1, color);
        video_set_pixel(x + radius - j, y + height - radius + i - 1, color);
        
        // Нижний правый угол
        video_set_pixel(x + width - radius + i - 1, y + height - radius + j - 1, color);
        video_set_pixel(x + width - radius + j - 1, y + height - radius + i - 1, color);
    }
}

// Оптимизированная версия без sqrt
void video_fill_rounded_rect(int x, int y, int width, int height, int radius, uint8_t color) {
    // Заполняем центральную часть
    video_fill_rect(x + radius, y + radius, width - 2 * radius, height - 2 * radius, color);
    
    // Заполняем верхнюю и нижнюю прямые части
    video_fill_rect(x + radius, y, width - 2 * radius, radius, color);
    video_fill_rect(x + radius, y + height - radius, width - 2 * radius, radius, color);
    
    // Заполняем левую и правую прямые части
    video_fill_rect(x, y + radius, radius, height - 2 * radius, color);
    video_fill_rect(x + width - radius, y + radius, radius, height - 2 * radius, color);
    
    // Заполняем углы
    int r2 = radius * radius;
    for (int i = 0; i < radius; i++) {
        for (int j = 0; j < radius; j++) {
            if (i * i + j * j <= r2) {
                // Верхний левый угол
                video_set_pixel(x + radius - i - 1, y + radius - j - 1, color);
                // Верхний правый угол
                video_set_pixel(x + width - radius + i, y + radius - j - 1, color);
                // Нижний левый угол
                video_set_pixel(x + radius - i - 1, y + height - radius + j, color);
                // Нижний правый угол
                video_set_pixel(x + width - radius + i, y + height - radius + j, color);
            }
        }
    }
}

void video_draw_gradient(int x, int y, int width, int height, uint8_t start_color, uint8_t end_color, int vertical) {
    for (int i = 0; i < (vertical ? height : width); i++) {
        float t = (float)i / (vertical ? height : width);
        uint8_t r = (uint8_t)(color_palette[start_color][0] * (1 - t) + color_palette[end_color][0] * t);
        uint8_t g = (uint8_t)(color_palette[start_color][1] * (1 - t) + color_palette[end_color][1] * t);
        uint8_t b = (uint8_t)(color_palette[start_color][2] * (1 - t) + color_palette[end_color][2] * t);
        
        uint8_t color = video_color_rgb(r, g, b);
        
        if (vertical) {
            video_draw_rect(x, y + i, width, 1, color);
        } else {
            video_draw_rect(x + i, y, 1, height, color);
        }
    }
}

void video_drop_shadow(int x, int y, int width, int height, int radius, uint8_t shadow_color) {
    for (int i = 0; i < radius; i++) {
        int alpha = (radius - i) * 255 / radius;
        uint8_t color = video_color_blend(shadow_color, video_get_pixel(x + width + i, y + height + i), alpha);
        
        // Нижняя тень
        for (int j = -radius; j < width + radius; j++) {
            video_set_pixel(x + j, y + height + i, color);
        }
        
        // Правая тень
        for (int j = 0; j < height; j++) {
            video_set_pixel(x + width + i, y + j, color);
        }
    }
}

void video_draw_line(int x1, int y1, int x2, int y2, uint8_t color) {
    int dx = absolute_value(x2 - x1);
    int dy = absolute_value(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        video_set_pixel(x1, y1, color);
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

void video_draw_circle(int center_x, int center_y, int radius, uint8_t color) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;
    
    while (y >= x) {
        video_set_pixel(center_x + x, center_y + y, color);
        video_set_pixel(center_x - x, center_y + y, color);
        video_set_pixel(center_x + x, center_y - y, color);
        video_set_pixel(center_x - x, center_y - y, color);
        video_set_pixel(center_x + y, center_y + x, color);
        video_set_pixel(center_x - y, center_y + x, color);
        video_set_pixel(center_x + y, center_y - x, color);
        video_set_pixel(center_x - y, center_y - x, color);
        
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void video_fill_circle(int center_x, int center_y, int radius, uint8_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                video_set_pixel(center_x + x, center_y + y, color);
            }
        }
    }
}

void video_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint8_t color) {
    video_draw_line(x1, y1, x2, y2, color);
    video_draw_line(x2, y2, x3, y3, color);
    video_draw_line(x3, y3, x1, y1, color);
}

void video_fill_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint8_t color) {
    // Simple triangle filling algorithm
    int min_x = x1, max_x = x1;
    int min_y = y1, max_y = y1;
    
    if (x2 < min_x) min_x = x2; if (x2 > max_x) max_x = x2;
    if (x3 < min_x) min_x = x3; if (x3 > max_x) max_x = x3;
    if (y2 < min_y) min_y = y2; if (y2 > max_y) max_y = y2;
    if (y3 < min_y) min_y = y3; if (y3 > max_y) max_y = y3;
    
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            // Barycentric coordinates check
            int w1 = (x2 - x3)*(y - y3) - (y2 - y3)*(x - x3);
            int w2 = (x3 - x1)*(y - y1) - (y3 - y1)*(x - x1);
            int w3 = (x1 - x2)*(y - y2) - (y1 - y2)*(x - x2);
            
            if (w1 >= 0 && w2 >= 0 && w3 >= 0) {
                video_set_pixel(x, y, color);
            }
        }
    }
}

// ========== GUI ЭЛЕМЕНТЫ ==========

void video_draw_button(int x, int y, int width, int height, const char* text, 
                       uint8_t bg_color, uint8_t text_color, int is_pressed) {
    int offset = is_pressed ? 1 : 0;
    
    // Тень кнопки
    video_fill_rect(x + 2, y + 2, width, height, COLOR_DARK_GREY);
    
    // Основная часть кнопки
    video_fill_rounded_rect(x + offset, y + offset, width, height, 4, bg_color);
    
    // Подсветка
    video_draw_rounded_rect(x + offset, y + offset, width, height, 4, COLOR_WHITE);
    
    // Текст кнопки
    int text_x = x + (width - strlen(text) * FONT_WIDTH) / 2 + offset;
    int text_y = y + (height - FONT_HEIGHT) / 2 + offset;
    video_draw_string(text_x, text_y, text, text_color);
}

void video_draw_panel(int x, int y, int width, int height, const char* title, 
                      uint8_t bg_color, uint8_t border_color, int has_shadow) {
    if (has_shadow) {
        video_drop_shadow(x, y, width, height, 4, COLOR_DARK_GREY);
    }
    
    // Основная панель
    video_fill_rounded_rect(x, y, width, height, 8, bg_color);
    video_draw_rounded_rect(x, y, width, height, 8, border_color);
    
    // Заголовок панели
    if (title && title[0] != '\0') {
        video_fill_rect(x, y, width, 30, border_color);
        video_draw_string(x + 10, y + 8, title, COLOR_WHITE);
    }
}

void video_draw_progress_bar(int x, int y, int width, int height, int progress, 
                             uint8_t bg_color, uint8_t fill_color, uint8_t border_color) {
    if (progress < 0) progress = 0;
    if (progress > 100) progress = 100;
    
    // Фон прогресс-бара
    video_fill_rounded_rect(x, y, width, height, 3, bg_color);
    video_draw_rounded_rect(x, y, width, height, 3, border_color);
    
    // Заполненная часть
    int fill_width = (width - 4) * progress / 100;
    if (fill_width > 0) {
        video_fill_rounded_rect(x + 2, y + 2, fill_width, height - 4, 2, fill_color);
    }
    
    // Текст прогресса (если высота достаточна)
    if (height >= 12) {
        char progress_text[8];
        int_to_string(progress, progress_text, 10);
        strcat(progress_text, "%");
        
        int text_x = x + (width - strlen(progress_text) * FONT_WIDTH) / 2;
        int text_y = y + (height - FONT_HEIGHT) / 2;
        video_draw_string(text_x, text_y, progress_text, COLOR_WHITE);
    }
}

void video_draw_slider(int x, int y, int width, int value, 
                       uint8_t track_color, uint8_t slider_color, uint8_t border_color) {
    // Трек слайдера
    video_fill_rounded_rect(x, y + 7, width, 6, 3, track_color);
    video_draw_rounded_rect(x, y + 7, width, 6, 3, border_color);
    
    // Ползунок
    int slider_x = x + (width * value / 100) - 8;
    video_fill_rounded_rect(slider_x, y, 16, 20, 4, slider_color);
    video_draw_rounded_rect(slider_x, y, 16, 20, 4, border_color);
}

void video_draw_checkbox(int x, int y, int size, int checked, 
                         uint8_t bg_color, uint8_t check_color, uint8_t border_color) {
    // Фон чекбокса
    video_fill_rect(x, y, size, size, bg_color);
    video_draw_rect(x, y, size, size, border_color);
    
    // Галочка
    if (checked) {
        for (int i = 2; i < size - 2; i++) {
            video_set_pixel(x + i, y + i, check_color);
            video_set_pixel(x + size - i - 1, y + i, check_color);
        }
    }
}

void video_draw_textbox(int x, int y, int width, int height, const char* text, 
                        uint8_t bg_color, uint8_t text_color, uint8_t border_color, int has_focus) {
    // Фон текстового поля
    video_fill_rect(x, y, width, height, bg_color);
    video_draw_rect(x, y, width, height, border_color);
    
    // Текст
    if (text && text[0] != '\0') {
        video_draw_string(x + 5, y + (height - FONT_HEIGHT) / 2, text, text_color);
    }
    
    // Курсор если в фокусе
    if (has_focus) {
        int cursor_x = x + 5 + strlen(text) * FONT_WIDTH;
        int cursor_y = y + (height - FONT_HEIGHT) / 2;
        video_draw_rect(cursor_x, cursor_y, 2, FONT_HEIGHT, text_color);
    }
}

void video_draw_scrollbar(int x, int y, int width, int height, int thumb_position, 
                          uint8_t bg_color, uint8_t thumb_color, uint8_t border_color) {
    // Фон скроллбара
    video_fill_rect(x, y, width, height, bg_color);
    video_draw_rect(x, y, width, height, border_color);
    
    // Бегунок
    int thumb_height = height / 3;
    int thumb_y = y + (height - thumb_height) * thumb_position / 100;
    video_fill_rect(x + 2, thumb_y, width - 4, thumb_height, thumb_color);
    video_draw_rect(x + 2, thumb_y, width - 4, thumb_height, border_color);
}

void video_draw_string_shadow(int x, int y, const char* str, uint8_t color, uint8_t shadow_color) {
    // Тень
    video_draw_string(x + 1, y + 1, str, shadow_color);
    // Основной текст
    video_draw_string(x, y, str, color);
}

// ========== СИСТЕМНЫЙ GUI (как на скриншоте) ==========

void video_draw_system_gui(void) {
    // Фон системы
    video_draw_gradient(0, 0, GFX_WIDTH, GFX_HEIGHT, COLOR_DARK_BLUE, COLOR_BLACK, 1);
    
    // Меню сверху
    video_draw_menu_bar();
    
    // Терминальное окно
    video_draw_terminal_window();
    
    // Панель системной информации
    video_draw_system_info_panel();
    
    // Статус бар
    video_draw_status_bar();
}

void video_draw_menu_bar(void) {
    int menu_height = 30;
    
    // Фон меню
    video_fill_rect(0, 0, GFX_WIDTH, menu_height, COLOR_DARK_BLUE);
    
    // Градиентная подсветка
    video_draw_gradient(0, 0, GFX_WIDTH, 2, COLOR_LIGHT_BLUE, COLOR_DARK_BLUE, 1);
    
    // Меню "File"
    video_draw_button(10, 5, 60, 20, "File", COLOR_BLUE, COLOR_WHITE, 0);
    // Меню "Edit"
    video_draw_button(80, 5, 60, 20, "Edit", COLOR_BLUE, COLOR_WHITE, 0);
    // Меню "View"
    video_draw_button(150, 5, 60, 20, "View", COLOR_BLUE, COLOR_WHITE, 0);
    // Меню "Terminal"
    video_draw_button(220, 5, 80, 20, "Terminal", COLOR_BLUE, COLOR_WHITE, 0);
    // Меню "System"
    video_draw_button(310, 5, 80, 20, "System", COLOR_BLUE, COLOR_WHITE, 0);
    
    // Кнопка сворачивания
    video_draw_button(GFX_WIDTH - 100, 5, 25, 20, "_", COLOR_GRAY, COLOR_WHITE, 0);
    // Кнопка разворачивания
    video_draw_button(GFX_WIDTH - 70, 5, 25, 20, "□", COLOR_GRAY, COLOR_WHITE, 0);
    // Кнопка закрытия
    video_draw_button(GFX_WIDTH - 40, 5, 25, 20, "X", COLOR_RED, COLOR_WHITE, 0);
}

void video_draw_terminal_window(void) {
    int term_x = 50;
    int term_y = 50;
    int term_width = 700;
    int term_height = 500;
    
    // Окно терминала с тенью
    video_draw_panel(term_x, term_y, term_width, term_height, 
                     "VIXOS Terminal - Nova Build", COLOR_BLACK, COLOR_LIGHT_GREY, 1);
    
    // Внутренняя область терминала
    video_fill_rect(term_x + 2, term_y + 32, term_width - 4, term_height - 34, COLOR_BLACK);
    
    // Текст терминала (как на скриншоте)
    int text_start_x = term_x + 10;
    int text_start_y = term_y + 50;
    
    video_draw_string(text_start_x, text_start_y, "Terminal", COLOR_LIGHT_GREEN);
    video_draw_string(text_start_x, text_start_y + 20, "VIXOS Code Name Nova Build", COLOR_LIGHT_CYAN);
    video_draw_string(text_start_x, text_start_y + 40, "C:/>_", COLOR_WHITE);
    
    // Статусная строка терминала
    video_fill_rect(term_x, term_y + term_height - 25, term_width, 25, COLOR_DARK_GREY);
    video_draw_string(term_x + 10, term_y + term_height - 18, "Ready", COLOR_LIGHT_GREEN);
    video_draw_string(term_x + term_width - 100, term_y + term_height - 18, "1024x768", COLOR_WHITE);
}

void video_draw_system_info_panel(void) {
    int panel_x = GFX_WIDTH - 300;
    int panel_y = 50;
    int panel_width = 250;
    int panel_height = 300;
    
    video_draw_panel(panel_x, panel_y, panel_width, panel_height, 
                     "System Information", COLOR_DARK_GREY, COLOR_LIGHT_BLUE, 1);
    
    int info_y = panel_y + 50;
    video_draw_string(panel_x + 10, info_y, "OS: VIXOS Nova", COLOR_WHITE);
    video_draw_string(panel_x + 10, info_y + 20, "Build: 20250510.1", COLOR_WHITE);
    video_draw_string(panel_x + 10, info_y + 40, "Resolution: 1024x768", COLOR_WHITE);
    video_draw_string(panel_x + 10, info_y + 60, "Color Depth: 256", COLOR_WHITE);
    video_draw_string(panel_x + 10, info_y + 80, "Memory: 16MB", COLOR_WHITE);
    
    // Прогресс-бар использования CPU
    video_draw_string(panel_x + 10, info_y + 110, "CPU Usage:", COLOR_WHITE);
    video_draw_progress_bar(panel_x + 10, info_y + 130, panel_width - 20, 15, 45, 
                           COLOR_DARK_GREY, COLOR_GREEN, COLOR_LIGHT_GREY);
    
    // Прогресс-бар использования памяти
    video_draw_string(panel_x + 10, info_y + 160, "Memory Usage:", COLOR_WHITE);
    video_draw_progress_bar(panel_x + 10, info_y + 180, panel_width - 20, 15, 65, 
                           COLOR_DARK_GREY, COLOR_BLUE, COLOR_LIGHT_GREY);
}

void video_draw_status_bar(void) {
    int status_height = 25;
    
    // Фон статус-бара
    video_fill_rect(0, GFX_HEIGHT - status_height, GFX_WIDTH, status_height, COLOR_DARK_GREY);
    
    // Разделители
    video_draw_rect(0, GFX_HEIGHT - status_height, GFX_WIDTH, status_height, COLOR_LIGHT_GREY);
    
    // Статусные индикаторы
    video_draw_string(10, GFX_HEIGHT - 18, "System: Running", COLOR_LIGHT_GREEN);
    video_draw_string(GFX_WIDTH - 200, GFX_HEIGHT - 18, "GPU: VGA 256-color", COLOR_WHITE);
    video_draw_string(GFX_WIDTH - 100, GFX_HEIGHT - 18, "Ready", COLOR_WHITE);
}

// ========== УТИЛИТЫ ЦВЕТА ==========

uint8_t video_color_rgb(uint8_t r, uint8_t g, uint8_t b) {
    // Поиск ближайшего цвета в палитре
    int min_distance = 256 * 256 * 3;
    uint8_t best_color = 0;
    
    for (int i = 0; i < 256; i++) {
        int dr = color_palette[i][0] - r;
        int dg = color_palette[i][1] - g;
        int db = color_palette[i][2] - b;
        int distance = dr * dr + dg * dg + db * db;
        
        if (distance < min_distance) {
            min_distance = distance;
            best_color = i;
        }
    }
    
    return best_color;
}

uint8_t video_color_blend(uint8_t color1, uint8_t color2, uint8_t alpha) {
    float a = alpha / 255.0;
    uint8_t r = (uint8_t)(color_palette[color1][0] * a + color_palette[color2][0] * (1 - a));
    uint8_t g = (uint8_t)(color_palette[color1][1] * a + color_palette[color2][1] * (1 - a));
    uint8_t b = (uint8_t)(color_palette[color1][2] * a + color_palette[color2][2] * (1 - a));
    
    return video_color_rgb(r, g, b);
}

void video_set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < 256) {
        color_palette[index][0] = r;
        color_palette[index][1] = g;
        color_palette[index][2] = b;
    }
}

uint8_t video_color_hsv(int hue, int saturation, int value) {
    // Convert HSV to RGB
    hue %= 360;
    if (hue < 0) hue += 360;
    
    int s = saturation * 255 / 100;
    int v = value * 255 / 100;
    
    int h = hue / 60;
    int f = (hue % 60) * 255 / 60;
    int p = (v * (255 - s)) / 255;
    int q = (v * (255 - f * s / 255)) / 255;
    int t = (v * (255 - (255 - f) * s / 255)) / 255;
    
    uint8_t r, g, b;
    switch (h) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
        default: r = v; g = v; b = v; break;
    }
    
    return video_color_rgb(r, g, b);
}

// ========== ПРОЧИЕ ФУНКЦИИ ==========

void video_switch_to_text(void) {
    graphics_mode = 0;
}

void video_switch_to_graphics(void) {
    graphics_mode = 1;
}

uint8_t* video_get_framebuffer(void) {
    if (double_buffering_enabled) {
        return back_buffer;
    } else {
        return (uint8_t*)GFX_MEMORY;
    }
}

int video_get_width(void) {
    return GFX_WIDTH;
}

int video_get_height(void) {
    return GFX_HEIGHT;
}

int video_get_graphics_mode(void) {
    return current_gfx_mode;
}

// Функции для работы со шрифтами
void video_draw_char(int x, int y, char c, uint8_t color) {
    if ((unsigned char)c >= FONT_CHARS) return;
    
    const uint8_t* font_data = default_font[(unsigned char)c];
    for (int row = 0; row < FONT_HEIGHT; row++) {
        uint8_t row_data = font_data[row];
        for (int col = 0; col < FONT_WIDTH; col++) {
            if (row_data & (1 << (7 - col))) {
                video_set_pixel(x + col, y + row, color);
            }
        }
    }
}

void video_draw_string(int x, int y, const char* str, uint8_t color) {
    int start_x = x;
    while (*str) {
        if (*str == '\n') {
            y += FONT_HEIGHT;
            x = start_x;
        } else {
            video_draw_char(x, y, *str, color);
            x += FONT_WIDTH;
        }
        str++;
    }
}

// Двойная буферизация
void video_enable_double_buffering(int enable) {
    double_buffering_enabled = enable;
    if (enable) {
        for (int i = 0; i < GFX_BUFFER_SIZE; i++) {
            back_buffer[i] = COLOR_BLACK;
        }
    }
}

void video_swap_buffers(void) {
    if (!double_buffering_enabled) return;
    
    for (int i = 0; i < GFX_BUFFER_SIZE; i++) {
        front_buffer[i] = back_buffer[i];
        GFX_MEMORY[i] = back_buffer[i];
    }
}

void video_clear_back_buffer(uint8_t color) {
    if (!double_buffering_enabled) return;
    
    for (int i = 0; i < GFX_BUFFER_SIZE; i++) {
        back_buffer[i] = color;
    }
}

// Функция печати целых чисел
void video_print_int(int num) {
    char buffer[12];
    int_to_string(num, buffer, 10);
    video_print(buffer);
}