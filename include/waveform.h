#ifndef WAVE_H
#define WAVE_H
#include "typedef.h"
#include <stdbool.h>

#define SAMPLE_RATE 44100
#define VOICE_MAX 8

struct voice {
    i32 midi_key;
    bool active;
    f64 time;
    f64 freq;
};

void voices_initialize(struct voice voices[VOICE_MAX]);
void voice_set_iterate(struct voice voices[VOICE_MAX], i32 midi_key, f64 freq);
void voice_clear_iterate(struct voice voices[VOICE_MAX], i32 midi_key);
f64 sawtooth(f64 time, f64 freq); 

#endif