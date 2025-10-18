#include "login.h"
#include "video.h"
#include "string.h"
#include "keyboard.h"
#include "sys.h"

#define USERNAME "vix"
#define PASSWORD "vixos123"

typedef struct {
    char text[32];
    int selected;
} MenuItem;

// Глобальные переменные для меню
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
        
        // Обработка ESC
        if (c == 0x1B) { // ESC key
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
    
    // Цветной заголовок (синим цветом)
    video_set_color(COLOR_WHITE, COLOR_BLUE);
    video_print(" ViXOS Welcome ");
    video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    video_print("\n\n");
    
    // Отображение меню с выделением выбранного пункта
    for (int i = 0; i < menu_count; i++) {
        if (welcome_menu[i].selected) {
            video_set_color(COLOR_BLACK, COLOR_WHITE); // Выделенный пункт
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

void handle_welcome_menu() {
    int running = 1;
    
    while (running) {
        char key = keyboard_getchar();
        
        switch (key) {
            case 0x48: // Стрелка вверх
            case 'w':
            case 'W':
                welcome_menu[current_selection].selected = 0;
                current_selection = (current_selection > 0) ? current_selection - 1 : menu_count - 1;
                welcome_menu[current_selection].selected = 1;
                display_welcome_menu();
                break;
                
            case 0x50: // Стрелка вниз
            case 's':
            case 'S':
                welcome_menu[current_selection].selected = 0;
                current_selection = (current_selection < menu_count - 1) ? current_selection + 1 : 0;
                welcome_menu[current_selection].selected = 1;
                display_welcome_menu();
                break;
                
            case '\n': // Enter
            case '\r':
                running = 0;
                switch (current_selection) {
                    case 0: // Login
                        login_prompt();
                        // После возврата из login_prompt обновляем меню
                        display_welcome_menu();
                        break;
                    case 1: // Reboot
                        reboot();
                        break;
                    case 2: // Shutdown
                        shutdown();
                        break;
                }
                break;
                
            default:
                break;
        }
    }
}

void display_menu() {
    video_clear();
    // Цветной заголовок (синим цветом)
    video_set_color(COLOR_WHITE, COLOR_LIGHT_BLUE);
    video_print(" Welcome to ViXOS ");
    video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    video_print("\n\n");

    // Отображение меню
    video_print("1. Login\n");
    video_print("2. Reboot\n");
    video_print("3. Shutdown\n");
    video_print("\nSelect an option (1-3): ");
}

void handle_menu_selection() {
    while (1) {
        char selection = keyboard_getchar();
        switch (selection) {
            case '1':   
                login_prompt();
                display_welcome_menu();
                return;
            case '2':
                reboot();
                break;
            case '3':
                shutdown();
                break;
            default:
                break; 
        }
    }
}   

void login_prompt() {
    char password[32];
    int authenticated = 0;
    int attempts = 0;

    while (!authenticated) {
        video_clear();

        video_set_color(COLOR_WHITE, COLOR_LIGHT_BLUE);
        video_print(" ViXOS Login ");
        video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
        video_print("\n\n");

        video_print("Username: ");
        video_print(USERNAME); // Показываем предустановленное имя пользователя
        video_print("\n");

        video_print("Password: ");
        read_line(password, sizeof(password), 1);

        // Проверка на нажатие ESC - возвращаемся в главное меню
        if (password[0] == 0) { // Если строка пустая (ESC был нажат)
            return; // Возвращаемся в главное меню
        }

        if (strcmp(password, PASSWORD) == 0) {
            authenticated = 1;
        } else {
            attempts++;
            video_print("\nIncorrect password!\n");
            
            // Сообщение для разработчиков после нескольких попыток
            if (attempts >= 2) {
                video_print("\nHey pss this system is not for the public and we do not recommend releasing development versions to the public.\n");
            }
            
            video_print("\nPress Enter to try again...\n");
            keyboard_getchar();
        }
    }

    video_clear();
    video_set_color(COLOR_GREEN, COLOR_BLACK);
    video_print(" Access Granted! Welcome to ViXOS ");
    video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    video_print("\n\n");
}