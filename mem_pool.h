#ifndef PARALELCOMPUTING_MEM_POOL_H
#define PARALELCOMPUTING_MEM_POOL_H

#include <stddef.h>

typedef struct MemPool MemPool;

/* Create a pool that manages 'total_blocks' blocks of 'block_size' bytes.
   Returns NULL on allocation failure. */
MemPool* pool_create(size_t block_size, unsigned int total_blocks);

/* Allocate one block from the pool. Returns NULL if none available. */
void* pool_alloc(MemPool* pool);

/* Free a previously allocated block back to the pool. Behavior undefined
   if ptr was not allocated from pool. */
void pool_free(MemPool* pool, void* ptr);

/* Destroy the pool and free all memory. */
void pool_destroy(MemPool* pool);


#endif //PARALELCOMPUTING_MEM_POOL_H