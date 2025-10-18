#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>

void irq_install();
void irq_set_handler(uint8_t irq, void (*handler)(void));
void irq_clear_mask(uint8_t irq);  // если ты добавил
#endif