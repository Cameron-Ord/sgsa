#ifndef UTIL_H
#define UTIL_H
#include "typedef.h"

#define DT(freq, samplerate) (freq) / (samplerate)
#define ARRLEN(array) sizeof((array)) / sizeof((array[0]))
// Ex. 44100 * 0.5 = 22050(500 MS)
#define MS_BUFSIZE(samplerate, alpha) (size_t)((samplerate) * (alpha))
f32 rand_range_f32(f32 x, f32 y);
f32 rand_f32(void);
f32 linear_interpolate(f32 target, f32 current, f32 alpha);
void* sgsa_free(void* mem);
f32 midi_to_base_freq(i32 n);
void* sgsa_malloc(size_t nmemb, size_t membsize);
#endif