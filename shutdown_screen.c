#include "shutdown_screen.h"
#include "video.h"
#include "timer.h"

void shutdown_screen() {
    video_clear();
    video_set_color(0x0F, 0); // Белый текст на черном фоне
    
    // Центрируем основной текст
    video_set_cursor(35, 10);
    video_print("ViXOS");
    
    // Текст состояния
    video_set_cursor(34, 12);
    video_print("Shutting down");
    
    // Анимация точек
    for (int i = 0; i < 10; i++) {
        video_set_cursor(47, 12);
        
        // Цикл анимации точек
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
    
    // Финальное сообщение
    video_clear();
    video_set_cursor(33, 12);
    video_print("It's now safe to turn off");
    video_set_cursor(38, 13);
    video_print("your computer");
    
    timer_wait(2000);
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
    for (int i = 0; i < 5; i++) {
        video_set_cursor(46, 12);
        
        // Цикл анимации точек
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
    
    // Сообщение о перезагрузке
    video_clear();
    video_set_cursor(37, 12);
    video_print("Rebooting");
    
    timer_wait(1000);
}