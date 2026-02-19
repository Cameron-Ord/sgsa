#ifndef UTIL_H
#define UTIL_H
#include "typedef.h"

#define DT(freq, samplerate) (freq) / (samplerate)
#define ARRLEN(array) sizeof((array)) / sizeof((array[0]))
// Ex. 44100 * 0.5 = 22050(500 MS)
#define MS_BUFSIZE(samplerate, alpha) (size_t)((samplerate) * (alpha))
f64 rand_range_f64(f64 x, f64 y);
f64 rand_f64(void);
f64 linear_interpolate(f64 target, f64 current, f64 alpha);
void sgsa_free(void *mem);
f64 midi_to_base_freq(i32 n);
void *sgsa_malloc(size_t nmemb, size_t membsize);
#endif