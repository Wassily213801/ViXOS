#include "port_io.h"
#include "video.h"
#include "keyboard.h"

void shutdown() {
    outw(0x2000, 0x604);
    outw(0x2000, 0x4004);
}

void reboot() {
    uint8_t temp;
    __asm__ volatile("cli");
    temp = inb(0x64);
    while (temp & 0x02) temp = inb(0x64);
    outb(0xFE, 0x64);
}

void safe_shutdown_screen() {
    video_clear();
    
    video_set_color(0x0F, 0x00);
    
    video_print("===============================================\n");
    video_print("              ViXOS Safe Shutdown\n");
    video_print("===============================================\n\n");
    
    video_set_color(0x0E, 0x00); // Желтый на черном для основного сообщения
    video_print("It is now safe to turn off your computer.\n\n");
    
    video_set_color(0x0B, 0x00); // Голубой на черном для инструкций
    video_print("If your computer does not support software\n");
    video_print("shutdown, please:\n\n");
    
    video_set_color(0x0A, 0x00); // Зеленый на черном для шагов
    video_print("1. Press and hold the power button until\n");
    video_print("   the computer turns off completely.\n\n");
    
    video_print("2. Alternatively, you can press Ctrl+Alt+Delete\n");
    video_print("   to restart the system.\n\n");
    
    video_set_color(0x0E, 0x00); // Желтый на черном для призыва к действию
    video_print("Press any key to return to terminal...\n");

    keyboard_getchar();
    
    video_clear();
    video_set_color(0x07, 0x00); // Возвращаем стандартный цвет (светло-серый на черном)
}