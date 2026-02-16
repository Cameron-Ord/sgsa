#ifndef WAVE_H
#define WAVE_H
#include "typedef.h"
#include <stdbool.h>
#include <stdarg.h>

#define PI 3.1415926535897932384626433832795
#define VOICE_MAX 6
#define MIN(x, y) ((x) < (y) ? (x) : (y))
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

#define ATTACK_TIME 0.1
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
    SQUARE_RAW,
    SINE_RAW,
    SAW_RAW,
    TRIANGLE_RAW,
    POLY_SAW,
    POLY_SQUARE,
    WAVE_FORM_END,
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
    f64 freq;
    f64 time;
    f64 amplitude;
};

struct voice {
    i32 midi_key;
    struct oscilator osc;
    struct envelope env;
};

struct voice_control {
    i32 waveform_id;
    struct internal_format fmt;
    struct voice voices[VOICE_MAX];
};


f64 adsr(i32 *state, f64 *envelope, const f64 *release, i32 samplerate);
f64 vibrato(f64 vrate, f64 depth, f64 freq, f64 samplerate);
f64 tremolo(f64 trate, f64 depth, f64 time);

f64 map_velocity(i32 second);
i32 next_waveform(const i32 current);
i32 prev_waveform(const i32 current);

struct envelope make_env(i32 state, f64 env, f64 release);
struct oscilator make_osciliator(f64 phase, f64 freq, f64 time, f64 amplitude);
struct internal_format make_format(u8 channels, i32 samplerate, u32 format);

void voice_set_osc(struct voice *v, struct oscilator fmt);
void voice_set_env(struct voice *v, struct envelope fmt);

void voice_set_iterate(struct voice voices[VOICE_MAX], i32 midi_key, struct oscilator osc, struct envelope env);
void voice_release_iterate(struct voice voices[VOICE_MAX], i32 midi_key, i32 samplerate);

void voices_initialize(struct voice voices[VOICE_MAX], struct oscilator osc, struct envelope env);

void vc_initialize(struct voice_control *vc, i32 wfid, struct internal_format fmt, struct oscilator osc, struct envelope env);
void vc_set_waveform(struct voice_control *vc, i32 wfid);
void vc_set_fmt(struct voice_control *vc, struct internal_format fmt);

// Raw waves
f64 sgn(f64 x, f64 duty);
f64 sawtooth(f64 amp, f64 phase); 
f64 square(f64 amp, f64 phase, f64 duty);
f64 triangle(f64 amp, f64 phase);
f64 sine(f64 amp, f64 phase);

// Polynomial blep methods
f64 polyblep(f64 dt, f64 phase);
f64 poly_square(f64 amp, f64 dt, f64 phase, f64 duty);
f64 poly_saw(f64 amp, f64 dt, f64 phase);
f64 poly_triangle(void);

// Unused additive methods
f64 fourier_pulse(f64 phase, f64 duty);
f64 fourier_square(f64 phase);
f64 fourier_sawtooth(f64 phase); 
f64 reverse_fourier_sawtooth(f64 phase);
#endif