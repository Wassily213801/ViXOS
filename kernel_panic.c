#include "kernel_panic.h"
#include "video.h"
#include "sys.h"
#include "string.h"
#include "terminal.h"

void panic(const char* message, int error_code) {
    // Сначала гарантированно переключаемся в текстовый режим
    asm volatile("cli"); // Отключаем прерывания
    
    // Форсируем текстовый режим VGA 80x25
    video_clear_with_color(COLOR_WHITE, COLOR_BLUE);
    video_set_color(COLOR_WHITE, COLOR_BLUE);
    video_set_cursor(0, 0);

    video_print(":(\n");
    video_print("ViXOS Kernel Panic\n");
    video_print("Error: ");
    video_print(message);
    video_print("\n\n");
    
    video_print("Code: 0x");
    char hex_buf[9];
    // Конвертируем error_code в hex
    for (int i = 7; i >= 0; i--) {
        int nibble = (error_code >> (i * 4)) & 0xF;
        hex_buf[7 - i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
    }
    hex_buf[8] = '\0';
    video_print(hex_buf);
    
    // Бесконечный цикл с запретом прерываний
    while (1) {
        asm volatile("hlt");
    }
}

void panic_command(const char* arg) {
    int error_code = 0;
    if (arg && strlen(arg) > 0) {
        error_code = atoi(arg);
    }
    panic("Unexpected exception", error_code);
}