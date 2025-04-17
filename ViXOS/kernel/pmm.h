#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PMM_BLOCK_SIZE 4096
#define PMM_MAX_BLOCKS 32768

void pmm_init();
void* pmm_alloc_block();
void pmm_free_block(void* block);

#endif