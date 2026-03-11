#ifndef UTIL_HPP
#define UTIL_HPP
#include "typedef.hpp"
#include <vector>

#define ARR_LEN(arr) sizeof(arr) / sizeof(arr[0])

f32 midi_to_freq(i32 n);
void copy_char_buffer(const char *src, char *dst, size_t len);

u8 toggle_bit(u8 original, u8 pos);
u8 clear_bit(u8 original, u8 pos);
u8 set_bit(u8 original, u8 pos);
bool check_bit(u8 original, u8 mask, u8 want);

#endif
