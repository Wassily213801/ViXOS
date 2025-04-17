#include "timer.h"
#include "irq.h"
#include "port_io.h"
#include "video.h"

volatile unsigned int tick = 0;

static void timer_callback() {
    tick++;
    if (tick % 100 == 0) {
        video_print("Tick 100\n");
    }
}

void timer_install() {
    irq_set_handler(0, timer_callback);

    uint32_t freq = 100; // 100 Гц
    uint16_t divisor = 1193180 / freq;

    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}