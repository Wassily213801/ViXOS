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
#include "fat.h"
#include "audio.h"
#include "gui.h"
#include "ide.h" 
#include "vixfs.h"
#include "ahci.h"
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,
    0x00,
    -(0x1BADB002)
};

void kernel_main(void);

void start() {
    kernel_main();
}

void boot_screen();

void kernel_main() {
    boot_screen();
    idt_install();
    isr_install();
    irq_install();
    timer_install();    
    pmm_init();
    ide_init();  
    //vixfs_init();
    //ahci_init();  // Инициализируем AHCI
    //fat_init();
    //fat_list_root_directory();
    display_welcome_menu();
    handle_welcome_menu();
    terminal_init();
    terminal_run();
}