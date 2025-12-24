#ifndef PANIC_H
#define PANIC_H

void panic(const char* message, int error_code);
void panic_command(const char* arg);
void ahci_panic(int error_code, const char* message);  // Добавляем

#endif