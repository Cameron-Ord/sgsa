#ifndef UTIL_HPP
#define UTIL_HPP
#include "typedef.hpp"

#define ARR_LEN(arr) sizeof(arr) / sizeof(arr[0])

f32 midi_to_freq(i32 n);
f32 normalize_msg_bipolar(u32 value);
f32 normalize_msg(u32 value);

void copy_char_buffer(const char *src, char *dst, size_t len);
f32 rand_f32_range(f32 min, f32 max);
u8 toggle_bit(u8 original, u8 pos);
u8 clear_bit(u8 original, u8 pos);
u8 set_bit(u8 original, u8 pos);
bool check_bit(u8 original, u8 mask, u8 want);

#endif
