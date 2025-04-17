#include "login.h"
#include "video.h"
#include "string.h"
#include "keyboard.h"

#define USERNAME "vix"
#define PASSWORD "vixos123"

void read_line(char *buffer, int max, int hide_input) {
    int index = 0;
    while (index < max - 1) {
        char c = keyboard_getchar();
        if (c == '\n') break;
        if (c == '\b') {
            if (index > 0) {
                index--;
                video_backspace();
            }
        } else {
            buffer[index++] = c;
            if (!hide_input)
                video_putc(c);
            else
                video_putc('*');
        }
    }
    buffer[index] = 0;
    video_putc('\n');
}

void login_prompt() {
    char username[32];
    char password[32];
    int authenticated = 0;

    while (!authenticated) {
        video_print("Welcome to ViXOS\n");
        video_print("Username: ");
        read_line(username, sizeof(username), 0);

        video_print("Password: ");
        read_line(password, sizeof(password), 1);

        if (strcmp(username, USERNAME) == 0 && strcmp(password, PASSWORD) == 0) {
            authenticated = 1;
        } else {
            video_print("Access Denied\n\n");
        }
    }

    video_clear();
    video_print("Access Granted\n");
}