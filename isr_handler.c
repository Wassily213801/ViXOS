#include "video.h"

void isr_handler(unsigned int int_no) {
    video_print("Interrupt: ");
    video_putc('0' + int_no);
    video_putc('\n');
}