#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

void* simple_malloc(size_t size);
void simple_free(void* ptr);

#endif