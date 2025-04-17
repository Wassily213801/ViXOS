#include "terminal.h"
#include "pmm.h"
#include "keyboard.h"
#include "video.h"
#include "port_io.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "timer.h"
#include "login.h"
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,
    0x00,
    -(0x1BADB002)
};
void _start() {
    kernel_main();
}

void kernel_main() {
    idt_install();
    isr_install();
    irq_install();
    timer_install();    
    pmm_init();
    login_prompt(); 
    terminal_init(); 
    terminal_run();
    void* block = pmm_alloc_block();
}