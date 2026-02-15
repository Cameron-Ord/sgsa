#ifndef WAVE_H
#define WAVE_H
#include "typedef.h"
#include <stdbool.h>
#include <stdarg.h>

#define SAMPLE_RATE 44100
#define VOICE_MAX 8

enum ENVELOPE_STATES {
    ENVELOPE_ATTACK = 0,
    ENVELOPE_SUSTAIN, 
    ENVELOPE_DECAY,
    ENVELOPE_RELEASE,
    ENVELOPE_OFF,
};

#define ATTACK_TIME 0.1
#define DECAY_TIME 0.1
#define SUSTAIN_LEVEL 0.9
#define RELEASE_TIME 0.1
//  0 -> 1  0 -> 1 
// (VALUE - VALUE) / SAMPLES
#define ATTACK_INCREMENT (1.0 - 0.0) / (ATTACK_TIME * SAMPLE_RATE)
#define DECAY_INCREMENT (1.0 - SUSTAIN_LEVEL) / (DECAY_TIME * SAMPLE_RATE)
#define RELEASE_INCREMENT(envelope) (envelope) / (RELEASE_TIME * SAMPLE_RATE)

enum WAVEFORM_IDS {
    WAVE_FORM_BEGIN = 0,
    TRIANGLE,
    SQUARE,
    FOURIER_ST,
    R_FOURIER_ST,
    SINE,
    WAVE_FORM_END,
};

struct voice {
    i32 waveform_id;
    i32 midi_key;
    f64 phase;
    f64 freq;
    f64 envelope;
    f64 prev;
    i32 state;
    f64 release_increment;
};

f64 lp(f64 *prev, f64 x, f64 cutoff);
f64 vibrato(f64 vrate, f64 depth, f64 freq);
i32 next_waveform(const i32 current);
i32 prev_waveform(const i32 current);
f64 linear_interpolate(f64 target, f64 current, const f64 alpha);
void voices_set_waveform(struct voice voices[VOICE_MAX], i32 wfid);
void voices_initialize(struct voice voices[VOICE_MAX], i32 wfid);
void voice_set_iterate(struct voice voices[VOICE_MAX], i32 midi_key, f64 freq);
void voice_release_iterate(struct voice voices[VOICE_MAX], i32 midi_key);

f64 sgn(f64 x, f64 duty);
f64 sawtooth(f64 phase); 
f64 fourier_sawtooth(f64 phase, f64 freq); 
f64 reverse_fourier_sawtooth(f64 phase, f64 freq);
f64 square(f64 phase, f64 duty);
f64 triangle(f64 phase);
f64 sine(f64 phase);
#endif