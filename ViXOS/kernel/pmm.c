#include "pmm.h"
#include "string.h"

static uint8_t memory_bitmap[PMM_MAX_BLOCKS / 8];

void set_bit(uint32_t bit) {
    memory_bitmap[bit / 8] |= (1 << (bit % 8));
}

void clear_bit(uint32_t bit) {
    memory_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

uint8_t test_bit(uint32_t bit) {
    return memory_bitmap[bit / 8] & (1 << (bit % 8));
}

void pmm_init() {
    memset(memory_bitmap, 0, sizeof(memory_bitmap));
}

void* pmm_alloc_block() {
    for (uint32_t i = 0; i < PMM_MAX_BLOCKS; i++) {
        if (!test_bit(i)) {
            set_bit(i);
            return (void*)(i * PMM_BLOCK_SIZE);
        }
    }
    return 0;
}

void pmm_free_block(void* block) {
    uint32_t index = (uint32_t)block / PMM_BLOCK_SIZE;
    clear_bit(index);
}