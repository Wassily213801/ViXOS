#include "video.h"
#include "port_io.h"
#include "string.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define TEXT_MEMORY ((volatile uint16_t*)0xB8000)

// Graphics mode constants
#define GFX_WIDTH 640
#define GFX_HEIGHT 480
#define GFX_BPP 8
#define GFX_MEMORY ((volatile uint8_t*)0xE0000000)
#define GFX_BUFFER_SIZE (GFX_WIDTH * GFX_HEIGHT)

// Window and animation configuration
#define MAX_WINDOWS 10
#define MAX_WINDOW_WIDTH 200
#define MAX_WINDOW_HEIGHT 200
#define MAX_ANIMATIONS 10
#define MAX_ANIMATION_FRAMES 20
#define ANIMATION_FRAME_SIZE (32*32)

// Font configuration
#define FONT_WIDTH 8
#define FONT_HEIGHT 16
#define FONT_CHARS 256

static uint8_t color = 0x07;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static int graphics_mode = 0;
static int current_gfx_mode = GFX_MODE_640x480_8BIT;

// Double buffering
static uint8_t front_buffer[GFX_BUFFER_SIZE];
static uint8_t back_buffer[GFX_BUFFER_SIZE];
static int double_buffering_enabled = 0;

// Window management
typedef struct {
    int x, y;
    int width, height;
    char title[32];
    uint8_t buffer[MAX_WINDOW_WIDTH * MAX_WINDOW_HEIGHT];
    uint8_t visible;
    uint8_t has_border;
    uint8_t border_color;
    uint8_t background_color;
} GfxWindow;

static GfxWindow windows[MAX_WINDOWS];
static int window_count = 0;

// Animation management
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
} Animation;

static Animation animations[MAX_ANIMATIONS];
static int animation_count = 0;

// Font data (8x16 basic font) - simplified for compilation
static const uint8_t default_font[FONT_CHARS][FONT_HEIGHT] = {
    // Placeholder - in real implementation, include complete font data
    {0}
};

static int absolute_value(int n) {
    return n < 0 ? -n : n;
}

// Simple int to string implementation for compatibility
static void int_to_string(int num, char* buffer, int base) {
    char* p = buffer;
    char* p1, *p2;
    int digits = 0;
    
    // Handle 0 explicitly
    if (num == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }
    
    // Handle negative numbers
    if (num < 0 && base == 10) {
        *p++ = '-';
        num = -num;
    }
    
    // Process individual digits
    p1 = p;
    while (num != 0) {
        int remainder = num % base;
        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
        num = num / base;
    }
    *p-- = '\0';
    
    // Reverse the string
    p2 = p1;
    while (p2 < p) {
        char tmp = *p2;
        *p2++ = *p;
        *p-- = tmp;
    }
}

static void scroll_screen_if_needed(void) {
    if (cursor_y < VGA_HEIGHT) return;

    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            TEXT_MEMORY[(y - 1) * VGA_WIDTH + x] = TEXT_MEMORY[y * VGA_WIDTH + x];
        }
    }

    for (int x = 0; x < VGA_WIDTH; x++) {
        TEXT_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = ((uint16_t)color << 8) | ' ';
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

// Text mode functions
void video_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            TEXT_MEMORY[y * VGA_WIDTH + x] = ((uint16_t)color << 8) | ' ';
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void video_putc(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) cursor_x--;
        TEXT_MEMORY[cursor_y * VGA_WIDTH + cursor_x] = ((uint16_t)color << 8) | ' ';
    } else {
        TEXT_MEMORY[cursor_y * VGA_WIDTH + cursor_x] = ((uint16_t)color << 8) | (uint8_t)c;
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

void video_backspace(void) {
    video_putc('\b');
}

void video_set_color(uint8_t fg, uint8_t bg) {
    color = (fg & 0x0F) | ((bg & 0x0F) << 4);
}

// Graphics mode functions
void video_init_graphics(void) {
    graphics_mode = 1;
    current_gfx_mode = GFX_MODE_640x480_8BIT;
    for (int i = 0; i < GFX_BUFFER_SIZE; i++) {
        GFX_MEMORY[i] = 0;
        if (double_buffering_enabled) {
            front_buffer[i] = 0;
            back_buffer[i] = 0;
        }
    }
}

void video_set_graphics_mode(int mode) {
    current_gfx_mode = mode;
    // In a real implementation, you would program the VGA controller
    // to switch to the requested mode
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
    for (int i = x; i < x + width; i++) {
        for (int j = y; j < y + height; j++) {
            video_set_pixel(i, j, color);
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

// Double buffering functions
void video_enable_double_buffering(int enable) {
    double_buffering_enabled = enable;
    if (enable) {
        // Initialize back buffer
        for (int i = 0; i < GFX_BUFFER_SIZE; i++) {
            back_buffer[i] = 0;
        }
    }
}

void video_swap_buffers(void) {
    if (!double_buffering_enabled) return;
    
    // Copy back buffer to front buffer and display
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

// Font and text in graphics mode
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

// Enhanced window management functions
int video_create_window(int x, int y, int width, int height, const char* title) {
    if (window_count >= MAX_WINDOWS || 
        width <= 0 || width > MAX_WINDOW_WIDTH || 
        height <= 0 || height > MAX_WINDOW_HEIGHT) {
        return -1;
    }
    
    GfxWindow* win = &windows[window_count];
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->visible = 1;
    win->has_border = 1;
    win->border_color = COLOR_WHITE;
    win->background_color = COLOR_BLACK;
    
    if (title) {
        // Simple string copy without strncpy
        const char* src = title;
        char* dst = win->title;
        int i = 0;
        while (*src && i < sizeof(win->title) - 1) {
            *dst++ = *src++;
            i++;
        }
        *dst = '\0';
    } else {
        win->title[0] = '\0';
    }
    
    // Clear window buffer
    for (int i = 0; i < width * height; i++) {
        win->buffer[i] = win->background_color;
    }
    
    return window_count++;
}

void video_destroy_window(int win_id) {
    if (win_id < 0 || win_id >= window_count) return;
    
    for (int i = win_id; i < window_count - 1; i++) {
        windows[i] = windows[i + 1];
    }
    window_count--;
}

void video_draw_window(int win_id) {
    if (win_id < 0 || win_id >= window_count || !windows[win_id].visible) return;
    
    GfxWindow* win = &windows[win_id];
    
    // Draw window background
    for (int y = 0; y < win->height; y++) {
        for (int x = 0; x < win->width; x++) {
            if (win->x + x >= 0 && win->x + x < GFX_WIDTH &&
                win->y + y >= 0 && win->y + y < GFX_HEIGHT) {
                video_set_pixel(win->x + x, win->y + y, win->buffer[y * win->width + x]);
            }
        }
    }
    
    // Draw border if enabled
    if (win->has_border) {
        video_draw_rect(win->x, win->y, win->width, win->height, win->border_color);
        
        // Draw title if exists
        if (win->title[0] != '\0') {
            video_draw_string(win->x + 2, win->y + 2, win->title, win->border_color);
        }
    }
}

void video_set_window_pixel(int win_id, int x, int y, uint8_t color) {
    if (win_id < 0 || win_id >= window_count) return;
    GfxWindow* win = &windows[win_id];
    if (x >= 0 && x < win->width && y >= 0 && y < win->height) {
        win->buffer[y * win->width + x] = color;
    }
}

void video_fill_window(int win_id, uint8_t color) {
    if (win_id < 0 || win_id >= window_count) return;
    GfxWindow* win = &windows[win_id];
    for (int i = 0; i < win->width * win->height; i++) {
        win->buffer[i] = color;
    }
    win->background_color = color;
}

void video_move_window(int win_id, int x, int y) {
    if (win_id < 0 || win_id >= window_count) return;
    windows[win_id].x = x;
    windows[win_id].y = y;
}

void video_resize_window(int win_id, int width, int height) {
    if (win_id < 0 || win_id >= window_count || 
        width <= 0 || width > MAX_WINDOW_WIDTH || 
        height <= 0 || height > MAX_WINDOW_HEIGHT) return;
    
    GfxWindow* win = &windows[win_id];
    
    // Create temporary buffer for content preservation
    uint8_t temp_buffer[MAX_WINDOW_WIDTH * MAX_WINDOW_HEIGHT];
    for (int i = 0; i < win->width * win->height; i++) {
        temp_buffer[i] = win->buffer[i];
    }
    
    int old_width = win->width;
    int old_height = win->height;
    
    win->width = width;
    win->height = height;
    
    // Clear and copy old content
    video_fill_window(win_id, win->background_color);
    int copy_width = (width < old_width) ? width : old_width;
    int copy_height = (height < old_height) ? height : old_height;
    
    for (int y = 0; y < copy_height; y++) {
        for (int x = 0; x < copy_width; x++) {
            win->buffer[y * width + x] = temp_buffer[y * old_width + x];
        }
    }
}

void video_show_window(int win_id, int visible) {
    if (win_id < 0 || win_id >= window_count) return;
    windows[win_id].visible = visible ? 1 : 0;
}

void video_set_window_title(int win_id, const char* title) {
    if (win_id < 0 || win_id >= window_count) return;
    if (title) {
        // Simple string copy
        const char* src = title;
        char* dst = windows[win_id].title;
        int i = 0;
        while (*src && i < sizeof(windows[win_id].title) - 1) {
            *dst++ = *src++;
            i++;
        }
        *dst = '\0';
    }
}

void video_set_window_border(int win_id, int has_border, uint8_t border_color) {
    if (win_id < 0 || win_id >= window_count) return;
    windows[win_id].has_border = has_border ? 1 : 0;
    windows[win_id].border_color = border_color;
}

// Enhanced animation functions
int video_create_animation(int x, int y, int frame_count, int speed, int loop) {
    if (animation_count >= MAX_ANIMATIONS || frame_count <= 0 || frame_count > MAX_ANIMATION_FRAMES) {
        return -1;
    }
    
    Animation* anim = &animations[animation_count];
    anim->x = x;
    anim->y = y;
    anim->frame_count = frame_count;
    anim->current_frame = 0;
    anim->active = 0;
    anim->loop = loop;
    anim->speed = speed;
    anim->frame_delay = speed;
    anim->frame_counter = 0;
    
    return animation_count++;
}

void video_destroy_animation(int anim_id) {
    if (anim_id < 0 || anim_id >= animation_count) return;
    
    for (int i = anim_id; i < animation_count - 1; i++) {
        animations[i] = animations[i + 1];
    }
    animation_count--;
}

void video_set_animation_frame(int anim_id, int frame, const uint8_t* data) {
    if (anim_id < 0 || anim_id >= animation_count || 
        frame < 0 || frame >= animations[anim_id].frame_count) {
        return;
    }
    
    // Simple memory copy without memcpy
    for (int i = 0; i < ANIMATION_FRAME_SIZE; i++) {
        animations[anim_id].frames[frame][i] = data[i];
    }
}

void video_draw_animation_frame(int anim_id, int frame) {
    if (anim_id < 0 || anim_id >= animation_count || 
        frame < 0 || frame >= animations[anim_id].frame_count) {
        return;
    }
    
    Animation* anim = &animations[anim_id];
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (anim->x + x >= 0 && anim->x + x < GFX_WIDTH &&
                anim->y + y >= 0 && anim->y + y < GFX_HEIGHT) {
                video_set_pixel(anim->x + x, anim->y + y, 
                    anim->frames[frame][y * 32 + x]);
            }
        }
    }
}

void video_play_animation(int anim_id) {
    if (anim_id < 0 || anim_id >= animation_count) return;
    animations[anim_id].current_frame = 0;
    animations[anim_id].active = 1;
    animations[anim_id].frame_counter = 0;
}

void video_update_animations(void) {
    for (int i = 0; i < animation_count; i++) {
        if (animations[i].active) {
            animations[i].frame_counter++;
            if (animations[i].frame_counter >= animations[i].frame_delay) {
                video_draw_animation_frame(i, animations[i].current_frame);
                animations[i].current_frame++;
                animations[i].frame_counter = 0;
                
                if (animations[i].current_frame >= animations[i].frame_count) {
                    if (animations[i].loop) {
                        animations[i].current_frame = 0;
                    } else {
                        animations[i].active = 0;
                    }
                }
            }
        }
    }
}

void video_stop_animation(int anim_id) {
    if (anim_id < 0 || anim_id >= animation_count) return;
    animations[anim_id].active = 0;
}

void video_set_animation_position(int anim_id, int x, int y) {
    if (anim_id < 0 || anim_id >= animation_count) return;
    animations[anim_id].x = x;
    animations[anim_id].y = y;
}

void video_print_int(int num) {
    char buffer[12];
    int_to_string(num, buffer, 10);
    video_print(buffer);
}
