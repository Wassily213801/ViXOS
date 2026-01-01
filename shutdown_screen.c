#include "shutdown_screen.h"
#include "video.h"
#include "timer.h"
#include "sys.h"

void shutdown_screen() {
    video_clear();
    video_set_color(0x0F, 0); // Белый текст на черном фоне
    
    // Центрируем основной текст
    video_set_cursor(35, 10);
    video_print("ViXOS");
    
    // Текст состояния
    video_set_cursor(34, 12);
    video_print("Shutting down");
    
    // Анимация точек (упрощенная)
    for (int i = 0; i < 3; i++) {
        video_set_cursor(47, 12);
        for (int dot = 0; dot < 4; dot++) {
            video_set_cursor(47, 12);
            for (int d = 0; d < dot; d++) {
                video_print(".");
            }
            for (int s = dot; s < 3; s++) {
                video_print(" ");
            }
            timer_wait(200);
        }
    }
    
    // Пытаемся выключить компьютер
    shutdown();
    
    // Если компьютер не выключился, показываем финальное сообщение
    video_clear();
    // Очищаем экран и показываем минимальное сообщение
    video_clear();
    video_set_cursor(33, 12);
    video_print("It is now safe to turn off");
    video_set_cursor(38, 13);
    video_print("your computer");
    
    // Бесконечный цикл
    while (1) {
        __asm__ volatile("hlt");
    }
}

void reboot_screen() {
    video_clear();
    video_set_color(0x0F, 0); // Белый текст на черном фоне
    
    // Центрируем основной текст
    video_set_cursor(35, 10);
    video_print("ViXOS");
    
    // Текст состояния
    video_set_cursor(36, 12);
    video_print("Restarting");
    
    // Анимация точек
    for (int i = 0; i < 2; i++) {
        video_set_cursor(46, 12);
        for (int dot = 0; dot < 4; dot++) {
            video_set_cursor(46, 12);
            for (int d = 0; d < dot; d++) {
                video_print(".");
            }
            for (int s = dot; s < 3; s++) {
                video_print(" ");
            }
            timer_wait(200);
        }
    }
    
    // Вызываем перезагрузку
    reboot();
    
    // Если перезагрузка не сработала
    video_clear();
    video_set_cursor(35, 12);
    video_print("Reboot failed");
    
    video_set_cursor(33, 14);
    video_print("Press Ctrl+Alt+Delete");
    
    while (1) {
        __asm__ volatile("hlt");
    }
}