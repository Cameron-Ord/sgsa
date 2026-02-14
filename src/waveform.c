#include "../include/waveform.h"
#include <math.h>
#include <stdio.h>

const f64 PI = 3.1415926535897932384626433832795;

f64 fourier_sawtooth(f64 phase, f64 freq){
    const i32 nyquist = SAMPLE_RATE / 2;
    const i32 cutoff = (i32)((nyquist * 0.5) / freq);
    f64 sum = 0.0;
    for(i32 k = 1; k <= cutoff; k++){
        sum += pow(-1.0, k) * sin(2.0 * PI * k * phase) / k;
    }
    sum *= -(2 * 1.0 / PI);
    return sum;
}

f64 reverse_fourier_sawtooth(f64 phase, f64 freq){
    const i32 nyquist = SAMPLE_RATE / 2;
    const i32 cutoff = (i32)((nyquist * 0.5) / freq);
    f64 sum = 0.0;
    for(i32 k = 1; k <= cutoff; k++){
        sum += pow(-1.0, k) * sin(2.0 * PI * k * phase) / k;
    }
    sum *= (2 * 1.0 / PI);
    return sum;
}


// https://en.wikipedia.org/wiki/Sawtooth_wave
f64 sawtooth(f64 phase, f64 freq){
    return 2.0 * (phase - 0.5);
}

f64 sgn(f64 x){
    if(x > 0) return 1.0;
    if(x < 0) return -1.0;
    return 0.0;
}

f64 square(f64 phase, f64 freq){
    return sgn(cos(2.0 * PI * phase));
}

f64 triangle(f64 phase, f64 freq){
    return 2.0 * fabs(2.0 * (phase - 0.5)) - 1.0;
}

f64 sine(f64 phase, f64 freq){
    return 1.0 * sin(2.0 * PI * phase);
}

f64 linear_interpolate(f64 target, f64 current, const f64 alpha){
    return (target - current) * alpha;
}

void voices_set_waveform(struct voice voices[VOICE_MAX], wave waveform){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        v->waveform = waveform;
    }
}

void voices_initialize(struct voice voices[VOICE_MAX], wave waveform){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        v->waveform = waveform;
        v->state = ENVELOPE_OFF;
        v->freq = 0.0;
        v->midi_key = -1;
        v->phase = 0.0;
        v->envelope = 0.0;
    }
}

void voice_set_iterate(struct voice voices[VOICE_MAX], i32 midi_key, f64 freq){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        if(v->state == ENVELOPE_OFF){
            v->state = ENVELOPE_ATTACK;
            v->midi_key = midi_key;
            v->freq = freq;
            v->phase = 0.0;
            v->envelope = 0.0;
            v->release_increment = 0.0;
            return;
        }
    }
}

void voice_release_iterate(struct voice voices[VOICE_MAX], i32 midi_key){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        if(v->state != ENVELOPE_OFF && v->midi_key == midi_key){
            v->state = ENVELOPE_RELEASE;
            v->release_increment = RELEASE_INCREMENT(v->envelope);
            return;
        }
    }
}