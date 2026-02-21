#ifndef OSCILATOR_H
#define OSCILATOR_H
#include "typedef.h"
#include "../include/define.h"

struct wave_spec {
    f64 octave_increment;
    f64 coefficient;
    f64 volume;
    f64 detune;
};

struct envelope {
    i32 state;
    f64 envelope;
    f64 attack, decay, sustain, release;
};

struct oscilator {
    f64 generated[CHANNEL_MAX];
    f64 filtered_high[CHANNEL_MAX];
    f64 filtered_low[CHANNEL_MAX];
    f64 phase;
    f64 integrator;
    f64 dcx, dcy;
    f64 time;
    i32 waveform_id;
    struct wave_spec spec;
    struct envelope env;
};

struct layer {
    u32 oscilators;
    f64 base_freq;
    struct oscilator osc[OSCILATOR_MAX];
};

struct wave_spec make_spec(f64 oct, f64 coeff, f64 vol, f64 detune);
struct layer make_layer(u32 count, ...);
struct envelope make_env(f64 atk, f64 dec, f64 sus, f64 rel);

struct oscilator make_default_oscilator(i32 wfid);
struct oscilator make_custom_oscilator(i32 wfid, f64 atk, f64 dec, f64 sus, f64 rel, f64 octpos, f64 coeff, f64 vol, f64 detune);

void osc_change_id(struct oscilator *osc, i32 wfid);
void osc_update_spec(struct oscilator *osc,  f64 oct, f64 coeff, f64 vol, f64 detune);
void osc_update_envelope(struct oscilator *osc, f64 atk, f64 dec, f64 sus, f64 rel);


#endif