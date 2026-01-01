#include "video.h"
#include "timer.h"

void boot_screen() {
    video_clear();
    video_set_color(0x0B, 0);
    video_set_cursor(35, 10);
    video_print("ViXOS");
    video_set_color(0x0F, 0);
    video_set_cursor(37, 10);
    
    int bar_length = 12;
    int start_x = 33;
    int y_pos = 12;

    const char* msg = "System Loading";

    for (int iter = 0; iter <= bar_length; iter++) {
        int dots = iter % 4; // 0..3 dots
        video_set_cursor(start_x, y_pos);
        video_print(msg);

        for (int i = 0; i < dots; i++) {
            video_print(".");
        }
        for (int i = dots; i < 3; i++) {
            video_print(" ");
        }

        timer_wait(500);
    }

    timer_wait(1000);
}