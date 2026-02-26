#include "util.hpp"
#include <cmath>
#include <cstring>


f32 buffer_rms(size_t count, f32 *buffer){
    f32 sum = 0.0f;
    for(size_t i = 0; i < count; i++){
        sum += buffer[i] * buffer[i];
    }
    return sqrtf(sum / (f32)count);
}

void copy_char_buffer(const char *src, char *dst, size_t len){
    strcpy(dst, src);
    dst[len] = '\0';
}