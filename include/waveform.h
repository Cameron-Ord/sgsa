#ifndef WAVE_H
#define WAVE_H
#include "typedef.h"
#include <stdbool.h>
#include <stdarg.h>

#define PI 3.1415926535897932384626433832795
#define VOICE_MAX 4
#define OSCILATOR_MAX 2
#define MONO 1
#define STEREO 2

#define MIDI_VELOCITY_MAX 127
#define MIDI_NOTE_MAX 127

enum ENVELOPE_STATES {
    ENVELOPE_ATTACK = 0,
    ENVELOPE_SUSTAIN, 
    ENVELOPE_DECAY,
    ENVELOPE_RELEASE,
    ENVELOPE_OFF,
};

#define ATTACK_TIME 0.0
#define DECAY_TIME 0.1
#define SUSTAIN_LEVEL 0.1
#define RELEASE_TIME 0.1
//  0 -> 1  0 -> 1 
// (VALUE - VALUE) / SAMPLES
#define ATTACK_INCREMENT(samplerate) (1.0 - 0.0) / (ATTACK_TIME * (samplerate))
#define DECAY_INCREMENT(samplerate) (1.0 - SUSTAIN_LEVEL) / (DECAY_TIME * (samplerate))
#define RELEASE_INCREMENT(envelope, samplerate) (envelope) / (RELEASE_TIME * (samplerate))

enum WAVEFORM_IDS {
    WAVE_FORM_BEGIN = 0,
    PULSE_RAW,
    SAW_RAW,
    TRIANGLE_RAW,
    PULSE_POLY,
    SAW_POLY,
    TRIANGLE_POLY,
    WAVE_FORM_END,
};

struct wave_spec {
    f64 octave_increment;
    f64 coefficient;
    f64 volume;
    f64 detune;
};

struct envelope {
    i32 state;
    f64 envelope;
    f64 release_increment;
};

struct internal_format {
    u8 CHANNELS;
    i32 SAMPLE_RATE;
    u32 FORMAT;
};

struct oscilator {
    f64 phase;
    f64 integrator;
    f64 dcx, dcy;
    f64 time;
    i32 waveform_id;
    struct wave_spec spec;
};

struct layer {
    u32 oscilators;
    f64 base_freq;
    struct oscilator osc[OSCILATOR_MAX];
};

struct voice {
    bool active;
    i32 midi_key;
    f64 amplitude;
    struct layer l;
    struct envelope env;
};

struct voice_control {
    struct internal_format fmt;
    struct voice voices[VOICE_MAX];
    f64 dcblock;
};

f64 rand_range_f64(f64 x, f64 y);
f64 rand_f64(void);

f64 quantize(f64 x, i32 depth);
f64 adsr(i32 *state, f64 *envelope, const f64 *release, i32 samplerate);
f64 vibrato(f64 vrate, f64 depth, f64 freq, f64 samplerate);
f64 tremolo(f64 trate, f64 depth, f64 time);

f64 map_velocity(i32 second);
u32 next_layer(u32 current, u32 last);
u32 prev_layer(u32 current, u32 last);

void print_layer(const char *msg, struct layer l);
struct wave_spec make_wave_spec(f64 octave_increment, f64 coefficient, f64 volume, f64 detune);
struct layer set_layer_freq(struct layer, f64 freq);
struct layer make_layer(u32 count, ...);
struct envelope make_env(i32 state, f64 env, f64 release);
struct oscilator make_oscilator(i32 wfid, struct wave_spec spec);
struct internal_format make_format(u8 channels, i32 samplerate, u32 format);

void voice_set_layer(struct voice *v, struct layer l);
void voice_set_env(struct voice *v, struct envelope env);

void voice_set_iterate(struct voice voices[VOICE_MAX], f64 amp, i32 midi_key, struct layer l, struct envelope env);
void voice_release_iterate(struct voice voices[VOICE_MAX], i32 midi_key, i32 samplerate);

void voices_initialize(struct voice voices[VOICE_MAX], struct layer l, struct envelope env);

void vc_initialize(struct voice_control *vc, struct internal_format fmt, struct layer l, struct envelope env);
void vc_set_fmt(struct voice_control *vc, struct internal_format fmt);

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