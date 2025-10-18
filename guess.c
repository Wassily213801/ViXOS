#include "guess.h"
#include "video.h"
#include "string.h"
#include "stdlib.h"
#include "keyboard.h"

#define GUESS_MIN_NUMBER 1
#define GUESS_MAX_NUMBER 100
#define GUESS_MAX_ATTEMPTS 10

static unsigned int rand_seed = 12345;

int rand(void) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (unsigned int)(rand_seed / 65536) % 32768;
}

void srand(unsigned int seed) {
    rand_seed = seed;
}

static int secret_number = 0;
static int attempts_left = 0;
static int game_active = 0;

void guess_print(const char* str) {
    video_print(str);
}

void guess_print_number(int num) {
    char buffer[16];
    itoa(num, buffer, 10);
    guess_print(buffer);
}

void guess_new_game(void) {
    static int seed_counter = 0;
    srand(seed_counter++);
    
    secret_number = (rand() % (GUESS_MAX_NUMBER - GUESS_MIN_NUMBER + 1)) + GUESS_MIN_NUMBER;
    attempts_left = GUESS_MAX_ATTEMPTS;
    game_active = 1;
    
    guess_print("\nGuess the Number Game!\n");
    guess_print("I'm thinking of a number between ");
    guess_print_number(GUESS_MIN_NUMBER);
    guess_print(" and ");
    guess_print_number(GUESS_MAX_NUMBER);
    guess_print("\nYou have ");
    guess_print_number(GUESS_MAX_ATTEMPTS);
    guess_print(" attempts.\n");
    guess_print("guess> ");
}

void guess_handle_guess(int guess) {
    if (!game_active) {
        guess_print("Game not started. Type 'start' to begin.\n");
        guess_print("guess> ");
        return;
    }
    
    if (guess < GUESS_MIN_NUMBER || guess > GUESS_MAX_NUMBER) {
        guess_print("Please enter a number between ");
        guess_print_number(GUESS_MIN_NUMBER);
        guess_print(" and ");
        guess_print_number(GUESS_MAX_NUMBER);
        guess_print("\n");
        guess_print("guess> ");
        return;
    }
    
    attempts_left--;
    
    if (guess == secret_number) {
        guess_print("Congratulations! You guessed the number ");
        guess_print_number(secret_number);
        guess_print(" correctly!\n");
        guess_print("You used ");
        guess_print_number(GUESS_MAX_ATTEMPTS - attempts_left);
        guess_print(" attempts.\n");
        game_active = 0;
        guess_print("Type 'start' to play again or 'quit' to exit.\n");
        guess_print("guess> ");
        return;
    }
    
    if (attempts_left <= 0) {
        guess_print("Game over! The number was ");
        guess_print_number(secret_number);
        guess_print("\n");
        guess_print("Type 'start' to play again or 'quit' to exit.\n");
        game_active = 0;
        guess_print("guess> ");
        return;
    }
    
    if (guess < secret_number) {
        guess_print("Too low! ");
    } else {
        guess_print("Too high! ");
    }
    
    guess_print("Attempts left: ");
    guess_print_number(attempts_left);
    guess_print("\n");
    guess_print("guess> ");
}

void guess_handle_command(const char* cmd) {
    char buffer[128];
    strcpy(buffer, cmd);
    char* command = strtok(buffer, " ");
    char* arg = strtok(NULL, " ");
    
    if (!command) {
        guess_print("guess> ");
        return;
    }
    
    if (strcmp(command, "start") == 0) {
        guess_new_game();
    }
    else if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
        guess_print("Thanks for playing! Returning to terminal...\n");
        return;
    }
    else if (strcmp(command, "help") == 0) {
        guess_print("Guess the Number Game Commands:\n");
        guess_print("start - Start a new game\n");
        guess_print("quit  - Exit the game\n");
        guess_print("help  - Show this help\n");
        guess_print("number - Make a guess (1-100)\n");
        guess_print("guess> ");
    }
    else {
        // Try to parse as number
        int guess = atoi(command);
        if (guess != 0 || command[0] == '0') {
            guess_handle_guess(guess);
        } else {
            guess_print("Unknown command. Type 'help' for available commands.\n");
            guess_print("guess> ");
        }
    }
}

void guess_game(void) {
    guess_print("\n=== Guess the Number Game ===\n");
    guess_print("Type 'start' to begin a new game\n");
    guess_print("Type 'help' for game instructions\n");
    guess_print("Type 'quit' to exit the game\n");
    guess_print("guess> ");
    
    while (1) {
        char input[128];
        int index = 0;
        
        while (1) {
            char key = keyboard_getchar();
            
            if (key == '\n') {
                video_putc('\n');
                input[index] = '\0';
                break;
            }
            else if (key == '\b') {
                if (index > 0) {
                    index--;
                    video_backspace();
                }
            }
            else if (index < sizeof(input) - 1) {
                input[index++] = key;
                video_putc(key);
            }
        }
        
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            guess_print("Goodbye!\n");
            break;
        }
        
        guess_handle_command(input);
    }
}