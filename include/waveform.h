#ifndef WAVE_H
#define WAVE_H
#include "define.h"
#include "effect.h"
#include "configs.h"
#include "oscilator.h"

#include <stdbool.h>
#include <stdarg.h>

struct delay_line;

struct voice {
    bool active;
    i32 midi_key;
    f64 amplitude;
    struct layer l;
};

struct voice_control {
    // What it doesn't own (Must be assgined with vc_assign() funcs)
    // All initialized to 0.
    f32 *render_buffer;
    size_t rbuflen;
    // What it owns.
    struct configs cfg;
    struct delay_line dl;
    struct voice voices[VOICE_MAX];
    f64 dcblock;
};

f64 quantize(f64 x, i32 depth);
void adsr(struct envelope *env, i32 samplerate);
f64 vibrato(f64 vrate, f64 depth, f64 freq, f64 samplerate);
f64 tremolo(f64 trate, f64 depth, f64 time);

f64 map_velocity(i32 second);
u32 next_layer(u32 current, u32 last);
u32 prev_layer(u32 current, u32 last);

void voice_set_iterate(struct voice voices[VOICE_MAX], f64 amp, i32 midi_key);
void voice_release_iterate(struct voice voices[VOICE_MAX], i32 midi_key);
void voices_initialize(struct voice voices[VOICE_MAX]);

void vc_assign_render_buffer(struct voice_control *vc, f32 *buffer, size_t len);
void vc_initialize(struct voice_control *vc);

void print_layer(const char *msg, struct layer l);
void layers_set_adsr(struct voice voices[VOICE_MAX], f64 atk, f64 dec, f64 sus, f64 rel);
// Raw waves
f64 sawtooth(f64 amp, f64 phase); 
f64 square(f64 amp, f64 phase, f64 duty);
f64 triangle(f64 amp, f64 phase);
f64 sine(f64 amp, f64 phase);

// Polynomial blep methods
f64 polyblep(f64 dt, f64 phase);
f64 poly_square(f64 amp, f64 dt, f64 phase, f64 duty);
f64 poly_saw(f64 amp, f64 dt, f64 phase);
f64 poly_triangle(f64 amp, f64 dt, f64 phase, f64 *integrator, f64 *x, f64 *y, f64 block);

// Unused additive methods
f64 fourier_pulse(f64 phase, f64 duty);
f64 fourier_square(f64 phase);
f64 fourier_sawtooth(f64 phase); 
f64 reverse_fourier_sawtooth(f64 phase);
#endif