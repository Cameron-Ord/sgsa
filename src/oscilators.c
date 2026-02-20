#include "../include/oscilator.h"
#include "../include/util.h"

#include <stdarg.h>
#include <stdio.h>

// Used per oscilator
const f64 DEFAULT_ENV_ATTACK = 0.125;
const f64 DEFAULT_ENV_DECAY = 0.225;
const f64 DEFAULT_ENV_SUSTAIN = 0.675;
const f64 DEFAULT_ENV_RELEASE = 0.175;
const f64 ZEROED = 0.0;

const f64 MIN_VAL = 0.0;
const f64 MAX_VAL = 1.0;

const f64 DEFAULT_OCTAVE_SKIP = 1.0;
const f64 DEFAULT_CONTRIBUTION_VOLUME = 1.0;
const bool DEFAULT_DETUNE = false;
const f64 DEFAULT_COEFF = 0.125;

void osc_change_id(struct oscilator *osc, i32 wfid){
    osc->waveform_id = wfid;
}

void osc_update_spec(struct oscilator *osc,  f64 oct, f64 coeff, f64 vol, bool detuned){
    osc->spec = make_spec(oct, coeff, vol, detuned);
}

void osc_update_envelope(struct oscilator *osc, f64 atk, f64 dec, f64 sus, f64 rel){
    osc->env = make_env(atk, dec, sus, rel);
}

struct envelope make_env(f64 atk, f64 dec, f64 sus, f64 rel){
    return (struct envelope){
        .state = ENVELOPE_OFF, 
        .envelope = ZEROED, 
        .attack = atk, 
        .decay = dec, 
        .sustain = sus, 
        .release = rel
    };
}

struct oscilator make_custom_oscilator(i32 wfid, f64 atk, f64 dec, f64 sus, f64 rel, f64 octpos, f64 coeff, f64 vol, bool detuned){
    return (struct oscilator){ 
        .generated = { ZEROED, ZEROED },
        .filtered_high = { ZEROED, ZEROED },
        .filtered_low = { ZEROED, ZEROED },
        .phase = rand_range_f64(0.0, 1.0), 
        .integrator = ZEROED, 
        .dcx = ZEROED, 
        .dcy = ZEROED, 
        .time = ZEROED, 
        .waveform_id = wfid, 
        .spec = make_spec(octpos, coeff, vol, detuned),
        .env = make_env(atk, dec, sus, rel)
    };
}

struct oscilator make_default_oscilator(i32 wfid){
    return (struct oscilator){ 
        .generated = { ZEROED, ZEROED },
        .filtered_high = { ZEROED, ZEROED },
        .filtered_low = { ZEROED, ZEROED },
        .phase = rand_range_f64(0.0, 1.0), 
        .integrator = ZEROED, 
        .dcx = ZEROED, 
        .dcy = ZEROED, 
        .time = ZEROED, 
        .waveform_id = wfid, 
        .spec = make_spec(
            DEFAULT_OCTAVE_SKIP, 
            DEFAULT_COEFF, 
            DEFAULT_CONTRIBUTION_VOLUME, 
            DEFAULT_DETUNE
        ),
        .env = make_env(
            DEFAULT_ENV_ATTACK, 
            DEFAULT_ENV_DECAY, 
            DEFAULT_ENV_SUSTAIN, 
            DEFAULT_ENV_RELEASE
        )
    };
}

struct wave_spec make_spec(f64 oct, f64 coeff, f64 vol, bool detuned){
    return (struct wave_spec) { 
        .octave_increment = oct, 
        .coefficient = coeff, 
        .volume = vol, 
        .detune = detuned
    };
}

struct layer make_layer(u32 count, ...){
    struct layer l = {0};
    l.oscilators = count;
    l.base_freq = ZEROED;
    va_list args;
    va_start(args, count);
    for(u32 i = 0; i < count; i++){
        l.osc[i] = va_arg(args, struct oscilator);
    }
    va_end(args);
    return l;
}