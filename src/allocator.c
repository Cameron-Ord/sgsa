#include "../include/allocator.h"
#include "../include/errdef.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

// strerror is not guaranteed to be set.
// But what else am I gonna do?
struct allocation sgsa_realloc(void** original, size_t size) {
    if (!original || !*original || size < 1)
        return (struct allocation){NULL, BAD_PARAM};

    void* allocated = realloc(*original, size);
    if (!allocated) {
        printf("Memory allocation failed! -> %s\n", strerror(errno));
        return (struct allocation){NULL, NO_MEM};
    }
    return (struct allocation){allocated, OK};
}

struct allocation sgsa_calloc(size_t nmemb, size_t size) {
    if (nmemb < 1 || size < 1)
        return (struct allocation){NULL, BAD_PARAM};

    void* allocated = calloc(nmemb, size);
    if (!allocated) {
        printf("Memory allocation failed! -> %s\n", strerror(errno));
        return (struct allocation){NULL, NO_MEM};
    }
    return (struct allocation){allocated, OK};
}

struct allocation sgsa_malloc(size_t size) {
    if (size < 1)
        return (struct allocation){NULL, BAD_PARAM};

    void* allocated = malloc(size);
    if (!allocated) {
        printf("Memory allocation failed! -> %s\n", strerror(errno));
        return (struct allocation){NULL, NO_MEM};
    }
    return (struct allocation){allocated, OK};
}

void sgsa_free(void* ptr) {
    if (!ptr)
        return;
    free(ptr);
    ptr = NULL;
}

struct region arena_create(u64 elements){
    //u8 is 1 byte.
    struct allocation result = sgsa_malloc(elements * sizeof(u8));
    if(result.err != OK){
        return (struct region){ 0, 0, NULL };
    }
    u8 *block = (u8*)result.allocated;

    memset(block, 0, elements * sizeof(u8));   
    struct region memory = {
        0,
        elements,
        block,
    };
    return memory;
}

struct region arena_expand(struct region arena){
    const u64 elements = arena.capacity * 2;
    struct allocation result = sgsa_realloc((void**)&arena.data, elements);
    arena.data = result.allocated;
    return arena;
}

struct region arena_shrink(struct region arena){
    const u64 elements = arena.capacity / 2;
    struct allocation result = sgsa_realloc((void**)&arena.data, elements);
    arena.data = result.allocated;
    return arena;
}

void *arena_alloc(struct region *arena, u64 bytes){
    if(arena->offset + bytes >= arena->capacity){
        return NULL;
    }
    // https://en.wikipedia.org/wiki/Data_structure_alignment
    // inverse the (align - 1) and drop all non matching bits with the AND operator
    // rounds upwards to the next power of two thanks to how binary works.
    const u64 align = alignof(max_align_t);
    const u64 offset = (arena->offset + (align - 1)) & ~(align - 1);
    void *ptr = arena->data + offset;
    arena->offset = offset + bytes;
 
    return ptr;
}

void arena_destroy(struct region *arena){

}