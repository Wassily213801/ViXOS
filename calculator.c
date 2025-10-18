#include "terminal.h"
#include "string.h"
#include "video.h"
#include "calculator.h"
#include "keyboard.h"

int calc_atoi(const char* str) {
    int result = 0;
    int sign = 1;
    int i = 0;
    
    while (str[i] == ' ') i++;
    
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }
    
    while (str[i] != '\0') {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
            i++;
        } else {
            break;
        }
    }
    
    return result * sign;
}

void calc_itoa(int num, char* str, int base) {
    int i = 0;
    int is_negative = 0;
    
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }
    
    while (num != 0) {
        int digit = num % base;
        str[i++] = (digit > 9) ? (digit - 10) + 'a' : digit + '0';
        num /= base;
    }

    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';

    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void calculator() {
    char input[64];
    char num1_str[16], num2_str[16], op[2];
    int num1, num2, result;
    
    video_print("\nViX Calculator v1.0\n");
    video_print("===================\n");
    video_print("Enter expression (e.g., 5 + 3)\n");
    video_print("Operations: +, -, *, /\n");
    video_print("Type 'exit' to quit\n\n");
    
    while (1) {
        video_print("calc> ");
        
        int i = 0;
        while (1) {
            char key = keyboard_getchar();
            
            if (key == '\n') {
                video_putc('\n');
                input[i] = '\0';
                break;
            } else if (key == '\b') {
                if (i > 0) {
                    i--;
                    video_backspace();
                }
            } else if (i < sizeof(input) - 1) {
                input[i++] = key;
                video_putc(key);
            }
        }
        
        if (strcmp(input, "exit") == 0) {
            video_print("Calculator closed.\n");
            break;
        }
        if (strcmp(input, "") == 0) {
            continue;
        }
        
        int parsed = 0;
        char* token = strtok(input, " ");
        
        if (token) {
            strcpy(num1_str, token);
            parsed++;
            
            token = strtok(NULL, " ");
            if (token && strlen(token) == 1) {
                op[0] = token[0];
                op[1] = '\0';
                parsed++;
                
                token = strtok(NULL, " ");
                if (token) {
                    strcpy(num2_str, token);
                    parsed++;
                }
            }
        }
        
        if (parsed != 3) {
            video_print("Error: Use format: number operator number\n");
            continue;
        }
        
        num1 = calc_atoi(num1_str);
        num2 = calc_atoi(num2_str);
        
        switch (op[0]) {
            case '+':
                result = num1 + num2;
                break;
            case '-':
                result = num1 - num2;
                break;
            case '*':
                result = num1 * num2;
                break;
            case '/':
                if (num2 == 0) {
                    video_print("Error: Division by zero\n");
                    continue;
                }
                result = num1 / num2;
                break;
            default:
                video_print("Error: Unknown operator. Use +, -, *, /\n");
                continue;
        }
        
        char result_str[32];
        calc_itoa(result, result_str, 10);
        video_print("= ");
        video_print(result_str);
        video_print("\n");
    }
}

void calculator_command() {
    calculator();
}