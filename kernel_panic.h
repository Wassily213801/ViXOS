#ifndef PANIC_H
#define PANIC_H

void panic(const char* message, int error_code);
void panic_command(const char* arg);
int atoi(const char* str);  // Add our custom atoi declaration

#endif