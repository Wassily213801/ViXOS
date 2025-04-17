#include "video.h"
#include "port_io.h"
#include "string.h"

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define WHITE_ON_BLACK 0x0F

static int cursor_row = 0;
static int cursor_col = 0;
static volatile char* video = (volatile char*)VGA_ADDRESS;

void move_cursor() {
    unsigned short pos = cursor_row * VGA_WIDTH + cursor_col;
    outb(0x0F, 0x3D4);
    outb((uint8_t)(pos & 0xFF), 0x3D5);
    outb(0x0E, 0x3D4);
    outb((uint8_t)((pos >> 8) & 0xFF), 0x3D5);
}

void video_clear() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video[i * 2] = ' ';
        video[i * 2 + 1] = WHITE_ON_BLACK;
    }
    cursor_row = 0;
    cursor_col = 0;
    move_cursor();
}

void video_putc(char c) {
    if (c == '\n') {
        cursor_row++;
        cursor_col = 0;
    } else {
        int index = (cursor_row * VGA_WIDTH + cursor_col) * 2;
        video[index] = c;
        video[index + 1] = WHITE_ON_BLACK;
        cursor_col++;
        if (cursor_col >= VGA_WIDTH) {
            cursor_col = 0;
            cursor_row++;
        }
    }

    if (cursor_row >= VGA_HEIGHT) {
        video_clear();
    }

    move_cursor();
}

void video_print(const char* str) {
    while (*str) {
        video_putc(*str++);
    }
}

void video_backspace() {
    if (cursor_col > 0) {
        cursor_col--;
    } else if (cursor_row > 0) {
        cursor_row--;
        cursor_col = VGA_WIDTH - 1;
    }

    int index = (cursor_row * VGA_WIDTH + cursor_col) * 2;
    video[index] = ' ';
    video[index + 1] = WHITE_ON_BLACK;

    move_cursor();
}
