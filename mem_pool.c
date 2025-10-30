#include "mem_pool.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct MemPool {
    size_t block_size;
    unsigned int total_blocks;
    void* memory;       // contiguous memory region
    void* free_list;    // pointer to first free block
};

MemPool* pool_create(size_t block_size, unsigned int total_blocks) {
    if (block_size == 0 || total_blocks == 0) return NULL;
    // ensure block can hold a pointer for the free-list
    size_t real_block = block_size < sizeof(void*) ? sizeof(void*) : block_size;
    size_t total_size = real_block * total_blocks;
    void* mem = malloc(total_size);
    if (!mem) return NULL;

    MemPool* pool = malloc(sizeof(MemPool));
    if (!pool) { free(mem); return NULL; }

    pool->block_size = real_block;
    pool->total_blocks = total_blocks;
    pool->memory = mem;

    // initialize free list: each block stores pointer to next
    uintptr_t cur = (uintptr_t)mem;
    for (unsigned int i = 0; i < total_blocks; ++i) {
        void* next = (i + 1 < total_blocks) ? (void*)(cur + real_block) : NULL;
        // write next pointer into current block
        *((void**)cur) = next;
        cur += real_block;
    }
    pool->free_list = mem;
    return pool;
}

void* pool_alloc(MemPool* pool) {
    if (!pool || !pool->free_list) return NULL;
    void* block = pool->free_list;
    // pop head
    pool->free_list = *((void**)pool->free_list);
    // optionally zero memory: memset(block, 0, pool->block_size);
    return block;
}

void pool_free(MemPool* pool, void* ptr) {
    if (!pool || !ptr) return;
    // push block back to free list
    *(void**)ptr = pool->free_list;
    pool->free_list = ptr;
}

void pool_destroy(MemPool* pool) {
    if (!pool) return;
    free(pool->memory);
    free(pool);
}