#include "terminal.h"
#include "video.h"
#include "keyboard.h"
#include "sys.h"
#include "string.h"

#define MAX_INPUT 256

char input_buffer[MAX_INPUT];
int input_index = 0;

void terminal_init() {
    video_clear();
    video_print("ViXOS Terminal Ready\n> ");
}

void handle_command(const char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        video_print("Available commands:\n");
        video_print("help - Show this message\n");
        video_print("hello - Print greeting\n");
        video_print("clear - Clear screen\n");
        video_print("version - Show system version\n");
        video_print("off - Power off\n");
        video_print("restart - Reboot system\n");
    } else if (strcmp(cmd, "hello") == 0) {
        video_print("Hello from ViXOS!\n");
    } else if (strcmp(cmd, "clear") == 0) {
        video_clear();
    } else if (strcmp(cmd, "version") == 0) {
        video_print("ViXOS Version 0.1\n");
    } else if (strcmp(cmd, "off") == 0) {
        shutdown();
    } else if (strcmp(cmd, "restart") == 0) {
        reboot();
    } else {
        video_print("Unknown command\n");
    }
    video_print("> ");
    input_index = 0;
    input_buffer[0] = '\0';
}

void terminal_run() {
    while (1) {
        char key = keyboard_getchar();

        if (key == '\n' || key == '\r') {
            video_putc('\n');
            input_buffer[input_index] = '\0';
            handle_command(input_buffer);
        } else if (key == '\b') {
            if (input_index > 0) {
                input_index--;
                input_buffer[input_index] = '\0';
                video_backspace();
            }
        } else if (key >= 32 && key <= 126) {
            if (input_index < MAX_INPUT - 1) {
                input_buffer[input_index++] = key;
                input_buffer[input_index] = '\0';
                video_putc(key);
            }
        }
    }
}
