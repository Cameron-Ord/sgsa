#include "../include/waveform.h"
#include <math.h>

// https://en.wikipedia.org/wiki/Sawtooth_wave
f64 sawtooth(f64 time, f64 freq){
    const f64 period = 1.0 / freq;
    return 2.0 * (time / period - floor(0.5 + time / period));
}

// This wont actually be used its just a placeholder for the idea of what i 
// will have to implement.

// Kinda dreading this because if I want to generate a specific 
// key as long as it is held without blocking im gonna have to use threads.
// We'll see I guess.
void generate(const bool *held, i32 samplerate, i32 freq){
    i32 i = 0;
    while(held){
        const f64 time = (f64)i / samplerate;
        f64 wave = sawtooth(time, freq);
    }
}