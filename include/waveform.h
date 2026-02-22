#ifndef WAVE_H
#define WAVE_H
#include "configs.h"
#include "define.h"
#include "effect.h"
#include "oscilator.h"

#include <stdarg.h>
#include <stdbool.h>

struct delay_line;

struct voice {
    bool active;
    i32 midi_key;
    f32 amplitude;
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
    f32 dcblock;
};

void adsr(f32 *envelope, f32 *state, const f32 *attack, const f32 *decay,
          const f32 *sustain, const f32 *release, i32 samplerate);
f32 vibrato(f32 vrate, f32 depth, f32 freq, f32 samplerate);
f32 tremolo(f32 trate, f32 depth, f32 time);

f32 map_velocity(i32 second);
u32 next_layer(u32 current, u32 last);
u32 prev_layer(u32 current, u32 last);

void voice_set_iterate(struct voice voices[VOICE_MAX], f32 amp, i32 midi_key);
void voice_release_iterate(struct voice voices[VOICE_MAX], i32 midi_key);
void voices_initialize(struct voice voices[VOICE_MAX]);

void vc_assign_render_buffer(struct voice_control *vc, f32 *buffer, size_t len);
void vc_initialize(struct voice_control *vc);

void layers_set_adsr(struct voice voices[VOICE_MAX], f32 atk, f32 dec, f32 sus,
                     f32 rel);
// Raw waves
f32 sawtooth(f32 amp, f32 phase);
f32 square(f32 amp, f32 phase, f32 duty);
f32 triangle(f32 amp, f32 phase);
f32 sine(f32 amp, f32 phase);

// Polynomial blep methods
f32 polyblep(f32 inc, f32 phase);
f32 poly_square(f32 amp, f32 inc, f32 phase, f32 duty);
f32 poly_saw(f32 amp, f32 inc, f32 phase);
f32 poly_triangle(f32 amp, f32 inc, f32 phase, f32 *integrator, f32 *x, f32 *y,
                  f32 block);

// Unused additive methods
f32 fourier_pulse(f32 phase, f32 duty);
f32 fourier_square(f32 phase);
f32 fourier_sawtooth(f32 phase);
f32 reverse_fourier_sawtooth(f32 phase);
#endif