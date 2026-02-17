#include "../include/util.h"
#include <stdlib.h>
#include <stdio.h>

f64 rand_f64(void){
    return (f64)rand() / (f64)RAND_MAX;
}

f64 rand_range_f64(f64 x, f64 y){
    return x + (y - x) * rand_f64();
}

f64 linear_interpolate(f64 target, f64 current, f64 alpha){
    return (target - current) * alpha;
}

void *sgsa_malloc(size_t nmemb, size_t membsize){
    if(!nmemb || !membsize) return NULL;
    void *ptr = malloc(nmemb * membsize);
    if(!ptr){
        printf("Out of memory!\n");
        return NULL;
    }
    return ptr;
}

void sgsa_free(void *ptr){
    if(!ptr) return;
    free(ptr);
    ptr = NULL;
}