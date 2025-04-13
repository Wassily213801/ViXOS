#include "terminal.h"
#include "keyboard.h"
#include "video.h"
#include "port_io.h"
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,
    0x00,
    -(0x1BADB002)
};

void kernel_main() {
    terminal_init();
    terminal_run();
}
