#include "util.hpp"
#include <cmath>
#include <cstring>

const i32 A4 = 69;
const f32 A4f = 440.0f;
const f32 NOTES = 12.0f;

f32 midi_to_freq(i32 n) {
    return A4f * powf(2.0f, (f32)(n - A4) / NOTES);
}

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