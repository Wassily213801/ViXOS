#include "kernel_panic.h"
#include "video.h"
#include "sys.h"
#include "string.h"

void panic(const char* message, int error_code) {
    video_set_color(0x4F, 0x00); // White on red
    video_clear();
    
    video_print("\n\n           ViXOS KERNEL PANIC\n");
    video_print("               Something went wrong and the system has halted.\n");
    video_print("               Error: ");
    video_print(message);
    video_print("\n             Error code: ");
    
    char code_str[16];
    itoa(error_code, code_str, 10);
    video_print(code_str);
    
    video_print("\n\n          The system will now halt.\n");
    
    // Halt the system
    asm volatile("cli");
    asm volatile("hlt");
}

void panic_command(const char* arg) {
    int error_code = 0;
    if (arg && strlen(arg) > 0) {
        error_code = atoi(arg);
    }
    panic("                 Manual panic triggered by user", error_code);
}