#ifndef LOGIN_H
#define LOGIN_H

// Коды ошибок для системы входа
#define LOGIN_ERROR_NONE 0
#define LOGIN_ERROR_ESC_PRESSED 1
#define LOGIN_ERROR_MAX_ATTEMPTS 2
#define LOGIN_ERROR_INVALID_CREDENTIALS 3
#define LOGIN_ERROR_SYSTEM_FAILURE 4

int login_prompt();  // Изменено с void на int
void display_welcome_menu();
void draw_welcome_menu();
void update_welcome_selection(int old_selection, int new_selection);
void handle_welcome_menu();
void handle_menu_selection();
void display_menu();
void read_line(char *buffer, int max, int hide_input);
void handle_login_error(int error_code);  // Добавлено

#endif