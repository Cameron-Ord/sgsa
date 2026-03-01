#include "util.hpp"

#include <cmath>
#include <cstring>

f32 midi_to_freq(i32 n) {
    const i32 A4 = 69;
    const f32 A4f = 440.0f;
    const f32 NOTES = 12.0f;
    return A4f * powf(2.0f, (f32)(n - A4) / NOTES);
}

void copy_char_buffer(const char *src, char *dst, size_t len){
    strcpy(dst, src);
    dst[len] = '\0';
}


u8 toggle_bit(u8 original, u8 pos){
    return original ^= pos;
}

u8 clear_bit(u8 original, u8 pos){
    return original &= ~pos;
}

u8 set_bit(u8 original, u8 pos){
    return original |= pos;
}

bool check_bit(u8 original, u8 mask, u8 want){
    return (original & mask) == want; 
}
