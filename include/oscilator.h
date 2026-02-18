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

void osc_set_env(struct oscilator *osc, f64 atk, f64 dec, f64 sus, f64 rel);
void osc_set_detune(struct oscilator *osc, f64 detune);

struct wave_spec make_spec(void);
struct layer make_layer(u32 count, ...);
struct envelope make_env(void);
struct oscilator make_oscilator(i32 wfid);

struct oscilator make_poly_square(void);
struct oscilator make_poly_triangle(void);
struct oscilator make_poly_saw(void);

struct oscilator make_saw(void);
struct oscilator make_square(void);
struct oscilator make_triangle(void);


#endif