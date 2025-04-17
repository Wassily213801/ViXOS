#ifndef ISR_H
#define ISR_H

#include <stdint.h>

void isr_install();
void isr_register_handler(uint8_t n, void (*handler)(uint32_t));

#endif