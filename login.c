#include "login.h"
#include "video.h"
#include "string.h"
#include "keyboard.h"
#include "sys.h"
#include "terminal.h"
#include "kernel_panic.h"
#include "shutdown_screen.h"

#define USERNAME "vix"
#define PASSWORD "vixos123"
#define MAX_LOGIN_ATTEMPTS 5

typedef struct {
    char text[32];
    int selected;
} MenuItem;

MenuItem welcome_menu[3] = {
    {"Login", 1},
    {"Reboot", 0},
    {"Shutdown", 0}
};

int current_selection = 0;
int menu_count = 3;

void read_line(char *buffer, int max, int hide_input) {
    int index = 0;
    while (index < max - 1) {
        char c = keyboard_getchar();
        
        if (c == 0x1B) {
            buffer[0] = 0;
            return;
        }
        
        if (c == '\n' || c == '\r') break;
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

void display_welcome_menu() {
    video_clear();
    
    video_set_color(COLOR_WHITE, COLOR_BLUE);
    video_print(" ViXOS Welcome ");
    video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    video_print("\n\n");
    
    for (int i = 0; i < menu_count; i++) {
        if (welcome_menu[i].selected) {
            video_set_color(COLOR_BLACK, COLOR_WHITE);
            video_print("> ");
        } else {
            video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
            video_print("  ");
        }
        
        video_print(welcome_menu[i].text);
        video_print("\n");
    }
    
    video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    video_print("\nUse arrow keys to navigate, Enter to select");
}

void handle_login_error(int error_code) {
    char error_msg[128];
    
    switch (error_code) {
        case LOGIN_ERROR_ESC_PRESSED:
            strcpy(error_msg, "Login cancelled by user (ESC pressed)");
            break;
        case LOGIN_ERROR_MAX_ATTEMPTS:
            strcpy(error_msg, "Maximum login attempts exceeded");
            break;
        case LOGIN_ERROR_INVALID_CREDENTIALS:
            strcpy(error_msg, "Invalid credentials detected");
            break;
        case LOGIN_ERROR_SYSTEM_FAILURE:
            strcpy(error_msg, "Login system failure");
            break;
        default:
            strcpy(error_msg, "Unknown login error");
            break;
    }
    
    if (error_code != LOGIN_ERROR_ESC_PRESSED) {
        video_clear();
        video_set_color(COLOR_RED, COLOR_BLACK);
        video_print(" Login Error ");
        video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
        video_print("\n\n");
        
        video_print("Error: ");
        video_print(error_msg);
        video_print("\n\n");
        
        video_print("Error Code: ");
        char code_str[16];
        itoa(error_code, code_str, 10);
        video_print(code_str);
        video_print("\n\n");
        
        video_print("Returning to main menu...\n");
        
        for (int i = 0; i < 10000000; i++) {
            asm volatile("nop");
        }
        
        if (error_code == LOGIN_ERROR_SYSTEM_FAILURE) {
            panic(error_msg, error_code);
        }
    }
}

void handle_welcome_menu() {
    int running = 1;
    
    while (running) {
        display_welcome_menu();
        
        while (running) {
            int key = keyboard_getkey();
            
            switch (key) {
                case KEY_UP:
                case 'w':
                case 'W':
                    welcome_menu[current_selection].selected = 0;
                    current_selection = (current_selection > 0) ? current_selection - 1 : menu_count - 1;
                    welcome_menu[current_selection].selected = 1;
                    display_welcome_menu();
                    break;
                    
                case KEY_DOWN:
                case 's':
                case 'S':
                    welcome_menu[current_selection].selected = 0;
                    current_selection = (current_selection < menu_count - 1) ? current_selection + 1 : 0;
                    welcome_menu[current_selection].selected = 1;
                    display_welcome_menu();
                    break;
                    
                case '\n':
                case '\r':
                    switch (current_selection) {
                        case 0:
                            {
                                int login_result = login_prompt();
                                if (login_result == LOGIN_ERROR_NONE) {
                                    if (login_result == LOGIN_ERROR_NONE) {
                                        current_selection = 0;
                                        welcome_menu[0].selected = 1;
                                        welcome_menu[1].selected = 0;
                                        welcome_menu[2].selected = 0;
                                    }
                                }
                            }
                            break;
                        case 1:
                            // Используем reboot_screen() вместо reboot()
                            video_clear();
                            reboot_screen();
                            // После отображения экрана перезагрузки, выполняем саму перезагрузку
                            reboot();
                            break;
                        case 2:
                            // Используем shutdown_screen() вместо shutdown()
                            video_clear();
                            shutdown_screen();
                            // После отображения экрана выключения, выполняем выключение
                            shutdown();
                            break;
                    }
                    break;
                    
                default:
                    break;
            }
            
            if (key == '\n' || key == '\r') {
                break;
            }
        }
    }
}

int login_prompt() {
    char password[32];
    int authenticated = 0;
    int attempts = 0;

    while (!authenticated && attempts < MAX_LOGIN_ATTEMPTS) {
        video_clear();

        video_set_color(COLOR_WHITE, COLOR_LIGHT_BLUE);
        video_print(" ViXOS Login ");
        video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
        video_print("\n\n");

        video_print("Username: ");
        video_print(USERNAME);
        video_print("\n");

        video_print("Password: ");
        read_line(password, sizeof(password), 1);

        if (password[0] == 0) {
            handle_login_error(LOGIN_ERROR_ESC_PRESSED);
            return LOGIN_ERROR_ESC_PRESSED;
        }

        if (strcmp(password, PASSWORD) == 0) {
            authenticated = 1;
            video_clear();
            video_set_color(COLOR_GREEN, COLOR_BLACK);
            video_print(" Access Granted! Welcome to ViXOS ");
            video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
            video_print("\n\n");
            
            for (int i = 0; i < 3000000; i++) {
                asm volatile("nop");
            }
            
            terminal_init();
            terminal_run();
            
            return LOGIN_ERROR_NONE;
        } else {
            attempts++;
            video_print("\nIncorrect password! Attempt ");
            char attempt_str[16];
            itoa(attempts, attempt_str, 10);
            video_print(attempt_str);
            video_print(" of ");
            char max_attempts_str[16];
            itoa(MAX_LOGIN_ATTEMPTS, max_attempts_str, 10);
            video_print(max_attempts_str);
            video_print("\n");
            
            //if (attempts >= 2) {
               //video_print("\nWarning: This system is not for public use.\n");
                //video_print("Development versions should not be released.\n");
            //}
            
            if (attempts < MAX_LOGIN_ATTEMPTS) {
                video_print("\nPress Enter to try again or ESC to cancel...\n");
                
                char c = keyboard_getchar();
                if (c == 0x1B) {
                    handle_login_error(LOGIN_ERROR_ESC_PRESSED);
                    return LOGIN_ERROR_ESC_PRESSED;
                }
            }
        }
    }

    if (attempts >= MAX_LOGIN_ATTEMPTS) {
        video_print("\n\nMaximum login attempts reached!\n");
        handle_login_error(LOGIN_ERROR_MAX_ATTEMPTS);
        
        video_print("System locked for 10 seconds...\n");
        for (int i = 0; i < 100000000; i++) {
            asm volatile("nop");
        }
        
        return LOGIN_ERROR_MAX_ATTEMPTS;
    }

    handle_login_error(LOGIN_ERROR_SYSTEM_FAILURE);
    return LOGIN_ERROR_SYSTEM_FAILURE;
}

void display_menu() {
    video_clear();
    video_set_color(COLOR_WHITE, COLOR_LIGHT_BLUE);
    video_print(" Welcome to ViXOS ");
    video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    video_print("\n\n");

    video_print("1. Login\n");
    video_print("2. Reboot\n");
    video_print("3. Shutdown\n");
    video_print("\nSelect an option (1-3): ");
}

void handle_menu_selection() {
    while (1) {
        display_menu();
        char selection = keyboard_getchar();
        switch (selection) {
            case '1':   
                {
                    int result = login_prompt();
                }
                break;
            case '2':
                // Используем экран перезагрузки
                video_clear();
                reboot_screen();
                reboot();
                break;
            case '3':
                // Используем экран выключения
                video_clear();
                shutdown_screen();
                shutdown();
                break;
            default:
                break; 
        }
    }
}