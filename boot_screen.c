#include "video.h"
#include "timer.h"

void boot_screen() {
    video_clear();
    video_set_color(0x0F, 0);
    video_set_cursor(35, 10);
    video_print("ViXOS");
    video_set_color(0x0A, 0);
    video_set_cursor(37, 10);


    int bar_length = 12;
    int start_x = 33;
    int y_pos = 12;

    video_set_cursor(start_x, y_pos);
    video_print("[            ]"); 
    
    for (int progress = 0; progress <= bar_length; progress++) {
        video_set_cursor(start_x + 1, y_pos); 
        
        for (int i = 0; i < bar_length; i++) {
            if (i < progress) {
                video_print("#");
            } else {
                video_print(" ");
            }
        }
        
        timer_wait(300);
    }
    
    timer_wait(1000);
}