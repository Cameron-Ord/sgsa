#ifndef UTIL_HPP
#define UTIL_HPP
#include "typedef.hpp"
f32 buffer_rms(size_t count, f32 *buffer);
void copy_char_buffer(const char *src, char *dst, size_t len);
#endif