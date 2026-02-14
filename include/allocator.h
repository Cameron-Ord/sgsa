#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include "typedef.h"

#define KiB(x) ((size_t)(x) * 1024)
#define MiB(x) ((size_t)(x) * 1024 * 1024)
#define GiB(x) ((size_t)(x) * 1024 * 1024 * 1024)

struct allocation {
    void* allocated;
    i32 err;
};

struct region {
    u64 offset;
    u64 capacity;
    u8 *data;
};

struct region arena_create(u64 elements);
// Expand or contract by power of 2
struct region arena_expand(struct region arena);
struct region arena_shrink(struct region arena);

void *arena_alloc(struct region *arena, u64 bytes);
void arena_destroy(struct region *arena);

struct allocation sgsa_malloc(size_t size);
struct allocation sgsa_calloc(size_t nmemb, size_t size);
struct allocation sgsa_realloc(void** original, size_t size);
void sgsa_free(void* ptr);

#endif