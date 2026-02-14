#include "../include/waveform.h"
#include <math.h>
#include <string.h>

// https://en.wikipedia.org/wiki/Sawtooth_wave
f64 sawtooth(f64 time, f64 freq){
    const f64 period = 1.0 / freq;
    return 2.0 * (time / period - floor(0.5 + time / period));
}

void voices_initialize(struct voice voices[VOICE_MAX]){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        v->active = false;
        v->freq = 0.0;
        v->midi_key = -1;
        v->time = 0.0;
    }
}

void voice_set_iterate(struct voice voices[VOICE_MAX], i32 midi_key, f64 freq){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        if(!v->active){
            v->active = true;
            v->midi_key = midi_key;
            v->freq = freq;
            v->time = 0.0;
            return;
        }
    }
}

void voice_clear_iterate(struct voice voices[VOICE_MAX], i32 midi_key){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        if(v->active && v->midi_key == midi_key){
            v->active = false;
            v->midi_key = -1;
            v->freq = 0.0;
            v->time = 0.0;
        }
    }
}