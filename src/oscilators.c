#include "../include/oscilator.h"
#include "../include/util.h"

#include <stdarg.h>

// Used per oscilator
const f64 DEFAULT_ENV_ATTACK = 0.125;
const f64 DEFAULT_ENV_DECAY = 0.225;
const f64 DEFAULT_ENV_SUSTAIN = 0.675;
const f64 DEFAULT_ENV_RELEASE = 0.175;

const f64 DEFAULT_OCTAVE_SKIP = 1.0;
const f64 DEFAULT_CONTRIBUTION_VOLUME = 1.0;
const f64 DEFAULT_DETUNE = 1.0;

struct oscilator make_saw(void){
    return make_oscilator(SAW_RAW);
}

struct oscilator make_square(void){
    return make_oscilator(PULSE_RAW);
}

struct oscilator make_triangle(void){
    return make_oscilator(TRIANGLE_RAW);
}

struct oscilator make_poly_square(void){
    return make_oscilator(PULSE_POLY);
}

struct oscilator make_poly_triangle(void){
    return make_oscilator(TRIANGLE_POLY);
}

struct oscilator make_poly_saw(void){
    return make_oscilator(SAW_POLY);
}

struct envelope make_env(void){
    return (struct envelope){
        ENVELOPE_OFF, 
        0.0, 
        DEFAULT_ENV_ATTACK, 
        DEFAULT_ENV_DECAY, 
        DEFAULT_ENV_SUSTAIN, 
        DEFAULT_ENV_RELEASE
    };
}

struct oscilator make_oscilator(i32 wfid){
    return (struct oscilator){ 
        .phase = rand_range_f64(0.0, 1.0), 
        .integrator = 0.0, 
        .dcx = 0.0, 
        .dcy = 0.0, 
        .time = 0.0, 
        .waveform_id = wfid, 
        .spec = make_spec(),
        .env = make_env()
    };
}

struct wave_spec make_spec(void){
    return (struct wave_spec) { 
        DEFAULT_OCTAVE_SKIP, 
        0.0, 
        DEFAULT_CONTRIBUTION_VOLUME, 
        DEFAULT_DETUNE 
    };
}

struct layer make_layer(u32 count, ...){
    struct layer l = {0};
    l.oscilators = count;
    l.base_freq = 0.0;
    va_list args;
    va_start(args, count);
    for(u32 i = 0; i < count; i++){
        l.osc[i] = va_arg(args, struct oscilator);
    }
    va_end(args);
    return l;
}