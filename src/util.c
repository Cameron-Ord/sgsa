#include "../include/util.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const i32 A4 = 69;
const f32 A4f = 440.0f;
const f32 NOTES = 12.0f;

f32 rand_f32(void) { return (f32)rand() / (f32)RAND_MAX; }

f32 rand_range_f32(f32 x, f32 y) { return x + (y - x) * rand_f32(); }

f32 linear_interpolate(f32 target, f32 current, f32 alpha) { return (target - current) * alpha; }

void* sgsa_malloc(size_t nmemb, size_t membsize) {
    if (!nmemb || !membsize)
        return NULL;
    void* ptr = malloc(nmemb * membsize);
    if (!ptr) {
        printf("Out of memory!\n");
        return NULL;
    }
    return ptr;
}

void* sgsa_free(void* ptr) {
    if (!ptr)
        return NULL;
    free(ptr);
    ptr = NULL;
    return NULL;
}

f32 midi_to_base_freq(i32 n) { return A4f * powf(2.0f, (f32)(n - A4) / NOTES); }
