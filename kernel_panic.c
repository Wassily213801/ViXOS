#include "kernel_panic.h"
#include "video.h"
#include "sys.h"
#include "string.h"

void panic(const char* message, int error_code) {
    // Белый текст (15) на синем фоне (1)
    // Вместо 0x1F, передаём отдельные цвета
    video_set_color(COLOR_WHITE, COLOR_BLUE);
    video_clear_with_color(COLOR_WHITE, COLOR_BLUE);
    
    // Выводим новый дизайн паники
    video_print("ViXOS Kernel Panic :(\n");
    video_print("Error: ");
    video_print(message);
    video_print("\n\n");
    
    video_print("Code: ");
    char code_str[16];
    itoa(error_code, code_str, 10);
    video_print(code_str);
    
    video_print("\nSystem halted.\n");
    
    // Останавливаем систему
    asm volatile("cli");
    asm volatile("hlt");
}

void panic_command(const char* arg) {
    int error_code = 0;
    if (arg && strlen(arg) > 0) {
        error_code = atoi(arg);
    }
    panic("Unexpected exception", error_code);
}