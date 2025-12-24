#include "snake.h"
#include "video.h"
#include "keyboard.h"
#include "string.h"

#define SNAKE_WIDTH 40
#define SNAKE_HEIGHT 20
#define SNAKE_START_LENGTH 3
#define SNAKE_MAX_LENGTH 200
#define GAME_SPEED 150  // Скорость игры (меньше = быстрее)

typedef struct {
    int x, y;
} Point;

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

static Point snake[SNAKE_MAX_LENGTH];
static int snake_length;
static Direction current_dir;
static Direction next_dir;  // Буфер для следующего направления
static Point food;
static int score;
static int game_over;
static int paused;
static int speed;

// Улучшенный генератор случайных чисел
static int simple_rand(int max) {
    static unsigned int seed = 12345;
    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return seed % max;
}

// Инициализация игры
static void snake_init(void) {
    snake_length = SNAKE_START_LENGTH;
    current_dir = DIR_RIGHT;
    next_dir = DIR_RIGHT;
    score = 0;
    game_over = 0;
    paused = 0;
    speed = GAME_SPEED;
    
    // Инициализация змейки в центре поля
    int start_x = SNAKE_WIDTH / 2;
    int start_y = SNAKE_HEIGHT / 2;
    
    for (int i = 0; i < snake_length; i++) {
        snake[i].x = start_x - i;
        snake[i].y = start_y;
    }
    
    // Генерация первой еды
    food.x = simple_rand(SNAKE_WIDTH);
    food.y = simple_rand(SNAKE_HEIGHT);
}

// Генерация новой еды
static void generate_food(void) {
    int valid_position;
    int attempts = 0;
    
    do {
        valid_position = 1;
        food.x = simple_rand(SNAKE_WIDTH);
        food.y = simple_rand(SNAKE_HEIGHT);
        
        // Проверяем, чтобы еда не появилась на змейке
        for (int i = 0; i < snake_length; i++) {
            if (snake[i].x == food.x && snake[i].y == food.y) {
                valid_position = 0;
                break;
            }
        }
        
        attempts++;
        if (attempts > 500) {
            // Если не можем найти свободное место, размещаем вручную
            for (int y = 0; y < SNAKE_HEIGHT; y++) {
                for (int x = 0; x < SNAKE_WIDTH; x++) {
                    int free = 1;
                    for (int i = 0; i < snake_length; i++) {
                        if (snake[i].x == x && snake[i].y == y) {
                            free = 0;
                            break;
                        }
                    }
                    if (free) {
                        food.x = x;
                        food.y = y;
                        return;
                    }
                }
            }
            break;
        }
    } while (!valid_position);
}

// Отрисовка игрового поля
static void draw_game(void) {
    video_clear_with_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    video_set_color(COLOR_YELLOW, COLOR_BLACK);
    
    // Заголовок и счёт
    video_print_at("=== SNAKE GAME v0.2 ===", ALIGN_CENTER, ALIGN_TOP);
    
    video_set_cursor(0, 1);
    video_set_color(COLOR_LIGHT_CYAN, COLOR_BLACK);
    video_print("Score: ");
    video_print_int(score);
    
    video_set_cursor(20, 1);
    video_print("Length: ");
    video_print_int(snake_length);
    
    video_set_cursor(40, 1);
    if (paused) {
        video_set_color(COLOR_LIGHT_RED, COLOR_BLACK);
        video_print("PAUSED");
    } else if (game_over) {
        video_set_color(COLOR_LIGHT_RED, COLOR_BLACK);
        video_print("GAME OVER");
    } else {
        video_set_color(COLOR_LIGHT_GREEN, COLOR_BLACK);
        video_print("PLAYING");
    }
    
    // Рисуем верхнюю границу
    video_set_cursor(0, 3);
    video_set_color(COLOR_CYAN, COLOR_BLACK);
    for (int x = 0; x < SNAKE_WIDTH + 2; x++) {
        video_putc('#');
    }
    
    // Рисуем игровое поле
    for (int y = 0; y < SNAKE_HEIGHT; y++) {
        video_set_cursor(0, y + 4);
        video_putc('#'); // Левая граница
        
        for (int x = 0; x < SNAKE_WIDTH; x++) {
            int is_snake = 0;
            int is_head = 0;
            int is_food = 0;
            int snake_index = -1;
            
            // Проверяем змейку
            for (int i = 0; i < snake_length; i++) {
                if (snake[i].x == x && snake[i].y == y) {
                    is_snake = 1;
                    snake_index = i;
                    if (i == 0) is_head = 1;
                    break;
                }
            }
            
            // Проверяем еду
            if (food.x == x && food.y == y) {
                is_food = 1;
            }
            
            // Отрисовываем символ
            if (is_head) {
                video_set_color(COLOR_YELLOW, COLOR_BLACK);
                if (game_over) {
                    video_set_color(COLOR_LIGHT_RED, COLOR_BLACK);
                    video_putc('X');
                } else {
                    // Выбираем символ головы в зависимости от направления
                    char head_char = 'O';
                    switch (current_dir) {
                        case DIR_UP: head_char = '^'; break;
                        case DIR_DOWN: head_char = 'v'; break;
                        case DIR_LEFT: head_char = '<'; break;
                        case DIR_RIGHT: head_char = '>'; break;
                    }
                    video_putc(head_char);
                }
            } else if (is_snake) {
                // Разные цвета для разных частей змейки
                if (snake_index % 4 == 0) video_set_color(COLOR_LIGHT_GREEN, COLOR_BLACK);
                else if (snake_index % 4 == 1) video_set_color(COLOR_GREEN, COLOR_BLACK);
                else if (snake_index % 4 == 2) video_set_color(COLOR_LIGHT_CYAN, COLOR_BLACK);
                else video_set_color(COLOR_CYAN, COLOR_BLACK);
                video_putc('*');
            } else if (is_food) {
                video_set_color(COLOR_LIGHT_RED, COLOR_BLACK);
                video_putc('@');
            } else {
                video_set_color(COLOR_DARK_GREY, COLOR_BLACK);
                video_putc('.');
            }
        }
        
        video_set_color(COLOR_CYAN, COLOR_BLACK);
        video_putc('#'); // Правая граница
    }
    
    // Рисуем нижнюю границу
    video_set_cursor(0, SNAKE_HEIGHT + 4);
    for (int x = 0; x < SNAKE_WIDTH + 2; x++) {
        video_putc('#');
    }
    
    // Статус и управление
    video_set_cursor(0, SNAKE_HEIGHT + 6);
    video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    
    if (game_over) {
        video_set_color(COLOR_LIGHT_RED, COLOR_BLACK);
        video_print("GAME OVER! Final Score: ");
        video_print_int(score);
        video_print("\n");
        video_set_color(COLOR_YELLOW, COLOR_BLACK);
        video_print("Press SPACE to restart or ESC to quit\n");
    } else {
        video_print("Controls: ");
        video_set_color(COLOR_LIGHT_GREEN, COLOR_BLACK);
        video_print("WASD");
        video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
        video_print(" to move, ");
        video_set_color(COLOR_LIGHT_BLUE, COLOR_BLACK);
        video_print("SPACE");
        video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
        video_print(" to pause, ");
        video_set_color(COLOR_LIGHT_RED, COLOR_BLACK);
        video_print("ESC");
        video_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
        video_print(" to quit\n");
        
        video_print("Speed: ");
        video_print_int(GAME_SPEED - speed + 100);
        video_print("%");
    }
}

// Обновление состояния игры
static void update_game(void) {
    if (game_over || paused) return;
    
    // Применяем следующее направление (с проверкой на противоположное)
    if ((next_dir == DIR_UP && current_dir != DIR_DOWN) ||
        (next_dir == DIR_DOWN && current_dir != DIR_UP) ||
        (next_dir == DIR_LEFT && current_dir != DIR_RIGHT) ||
        (next_dir == DIR_RIGHT && current_dir != DIR_LEFT)) {
        current_dir = next_dir;
    }
    
    // Сохраняем предыдущую позицию хвоста для возможного роста
    Point prev_tail = snake[snake_length - 1];
    
    // Двигаем тело змейки
    for (int i = snake_length - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    
    // Двигаем голову
    switch (current_dir) {
        case DIR_UP:
            snake[0].y--;
            break;
        case DIR_DOWN:
            snake[0].y++;
            break;
        case DIR_LEFT:
            snake[0].x--;
            break;
        case DIR_RIGHT:
            snake[0].x++;
            break;
    }
    
    // Проверка выхода за границы (телепортация через стены)
    if (snake[0].x < 0) snake[0].x = SNAKE_WIDTH - 1;
    else if (snake[0].x >= SNAKE_WIDTH) snake[0].x = 0;
    
    if (snake[0].y < 0) snake[0].y = SNAKE_HEIGHT - 1;
    else if (snake[0].y >= SNAKE_HEIGHT) snake[0].y = 0;
    
    // Проверка столкновения с собой
    for (int i = 1; i < snake_length; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            game_over = 1;
            return;
        }
    }
    
    // Проверка съедания еды
    if (snake[0].x == food.x && snake[0].y == food.y) {
        // Увеличиваем змейку
        if (snake_length < SNAKE_MAX_LENGTH) {
            snake[snake_length] = prev_tail;
            snake_length++;
        }
        
        score += 10;
        
        // Увеличиваем скорость каждые 50 очков
        if (score % 50 == 0 && speed > 50) {
            speed -= 10;
        }
        
        generate_food();
    }
}

// Обработка ввода (неблокирующая)
static void handle_input(void) {
    char key = keyboard_getchar_noblock();

    if (key) {
        if (game_over) {
            if (key == ' ' || key == '\n' || key == '\r') {
                snake_init(); // Перезапуск игры
            }
        } else {
            switch (key) {
                case 'w':
                case 'W':
                    if (current_dir != DIR_DOWN) next_dir = DIR_UP;
                    break;
                case 's':
                case 'S':
                    if (current_dir != DIR_UP) next_dir = DIR_DOWN;
                    break;
                case 'a':
                case 'A':
                    if (current_dir != DIR_RIGHT) next_dir = DIR_LEFT;
                    break;
                case 'd':
                case 'D':
                    if (current_dir != DIR_LEFT) next_dir = DIR_RIGHT;
                    break;
                case ' ': // Пауза
                    paused = !paused;
                    break;
                case '+': // Увеличить скорость
                    if (speed > 50) speed -= 10;
                    break;
                case '-': // Уменьшить скорость
                    if (speed < 300) speed += 10;
                    break;
            }
        }
    }

    // Проверяем специальные клавиши (стрелки, ESC и т.д.) неблокирующе
    int special_key = keyboard_getkey_noblock();
    if (special_key != 0) {
        if (game_over) {
            if (special_key == ' ') {
                snake_init();
            }
        } else {
            switch (special_key) {
                case KEY_UP:
                    if (current_dir != DIR_DOWN) next_dir = DIR_UP;
                    break;
                case KEY_DOWN:
                    if (current_dir != DIR_UP) next_dir = DIR_DOWN;
                    break;
                case KEY_LEFT:
                    if (current_dir != DIR_RIGHT) next_dir = DIR_LEFT;
                    break;
                case KEY_RIGHT:
                    if (current_dir != DIR_LEFT) next_dir = DIR_RIGHT;
                    break;
                case ' ':
                    paused = !paused;
                    break;
            }
        }
    }
}

// Простая функция задержки
static void snake_delay(int ms) {
    for (volatile int i = 0; i < ms * 1000; i++) {
        __asm__ volatile("nop");
    }
}

// Главная функция игры
void snake_game(void) {
    snake_init();
    
    video_clear_with_color(COLOR_BLACK, COLOR_BLACK);
    video_set_color(COLOR_YELLOW, COLOR_BLACK);
    video_print_at("SNAKE GAME v0.2", ALIGN_CENTER, ALIGN_MIDDLE);
    video_print_at("Loading...", ALIGN_CENTER, ALIGN_BOTTOM);
    snake_delay(1000);
    
    int running = 1;
    
    while (running) {
        // Обработка ввода
        handle_input();
        
        // Проверка на выход (неблокирующая)
        {
            int k = keyboard_getkey_noblock();
            if (k == KEY_ESC) { // ESC
                running = 0;
                break;
            }
            // Если это был другой ключ — положим его обратно в обработку (не нужно, handle_input уже опрашивает буфер)
        }
        
        // Обновляем и рисуем игру
        update_game();
        draw_game();
        
        // Задержка для регулировки скорости
        snake_delay(speed);
    }
    
    video_clear_with_color(COLOR_BLACK, COLOR_BLACK);
    video_set_color(COLOR_LIGHT_GREEN, COLOR_BLACK);
    video_print_at("Thanks for playing!", ALIGN_CENTER, ALIGN_MIDDLE);
    video_print_at("Final score: ", ALIGN_CENTER, ALIGN_BOTTOM);
    video_print_int(score);
    snake_delay(2000);
}