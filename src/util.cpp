#include "util.hpp"
#include "sgsa.hpp"

#include <cmath>
#include <cstring>

bool is_generating(u8 flags){
    const bool first = check_bit(flags, VOICE_ON | ENVELOPE_OFF | ENVELOPE_RELEASING, VOICE_ON);
    const bool second = check_bit(flags, VOICE_OFF | ENVELOPE_RELEASING, VOICE_OFF | ENVELOPE_RELEASING);
    return first || second;
}

f32 midi_to_freq(i32 n) {
    const i32 A4 = 69;
    const f32 A4f = 440.0f;
    const f32 NOTES = 12.0f;
    return A4f * powf(2.0f, (f32)(n - A4) / NOTES);
}

f32 voice_rms(struct Voice *voices, i32 c){
    f32 sum = 0.0f;
    u8 count = 0;
    for(u8 i = 0; i < MAX_VOICE; i++){
        if(is_generating(voices[i].voice_state)){
            sum += voices[i].gen[c] * voices[i].gen[c];
            count++;
        }
    }
    if(count < 1){
        return 0.0f;
    }
    return sqrtf(sum / count);
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
