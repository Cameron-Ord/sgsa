#include "../include/oscilator.h"
#include "../include/util.h"

#include <stdarg.h>
#include <stdio.h>

// Used per oscilator
const f64 DEFAULT_ENV_ATTACK = 0.125;
const f64 DEFAULT_ENV_DECAY = 0.225;
const f64 DEFAULT_ENV_SUSTAIN = 0.675;
const f64 DEFAULT_ENV_RELEASE = 0.175;
const f64 ENVELOPE_ZEROED = 0.0;

const f64 DETUNE_UP = 1.004;
const f64 DETUNE_DOWN = 0.996;

const f64 MIN_VAL = 0.0;
const f64 MAX_VAL = 1.0;

const f64 DEFAULT_OCTAVE_SKIP = 1.0;
const f64 DEFAULT_CONTRIBUTION_VOLUME = 1.0;
const f64 DEFAULT_DETUNE = 1.0;
const f64 DEFAULT_COEFF = 0.125;

const struct wave_spec SPEC_PRESETS[] = {
{
        0.5,
        0.5,
        DEFAULT_CONTRIBUTION_VOLUME,
        DEFAULT_DETUNE
    },
{
        2.0,
        0.5,
        DEFAULT_CONTRIBUTION_VOLUME,
        DEFAULT_DETUNE
    },
{
        DEFAULT_OCTAVE_SKIP,
        0.5,
        DEFAULT_CONTRIBUTION_VOLUME,
        DEFAULT_DETUNE
    },
    {
        DEFAULT_OCTAVE_SKIP,
        0.5,
        DEFAULT_CONTRIBUTION_VOLUME,
        DETUNE_DOWN
    },
    {
        DEFAULT_OCTAVE_SKIP,
        0.5,
        DEFAULT_CONTRIBUTION_VOLUME,
        DETUNE_UP
    },
};
const size_t SPEC_PRESET_SIZE = ARRLEN(SPEC_PRESETS);

const struct envelope ENVELOPE_PRESETS[] = {
{
        ENVELOPE_OFF, 
        ENVELOPE_ZEROED, 
        0.750, 
        1.0, 
        0.875, 
        1.5
    },
{
        ENVELOPE_OFF, 
        ENVELOPE_ZEROED, 
        0.03, 
        0.125, 
        0.75, 
        0.100
    },
{
        ENVELOPE_OFF, 
        ENVELOPE_ZEROED, 
        0.175, 
        0.675, 
        0.775, 
        1.0
    },
};
const size_t ENVELOPE_PRESET_SIZE = ARRLEN(ENVELOPE_PRESETS);


struct oscilator make_preset_oscilator(i32 wfid, size_t env_preset_index, size_t spec_preset_index){
    if(env_preset_index >= ENVELOPE_PRESET_SIZE){
        env_preset_index = ENVELOPE_PRESET_SIZE - 1;
    }
    const struct envelope *env = &ENVELOPE_PRESETS[env_preset_index];

    if(spec_preset_index >= SPEC_PRESET_SIZE){
        spec_preset_index =SPEC_PRESET_SIZE - 1;
    }
    const struct wave_spec *spec = &SPEC_PRESETS[spec_preset_index];

    printf("==Created oscilator==\n");
    printf("Using Spec: {(%.2f), (%.2f), (%.2f), (%.2f)}\n", spec->octave_increment, spec->coefficient, spec->volume, spec->detune);
    printf("Using Envelope: {(%.2f), (%.2f), (%.2f), (%.2f)}\n", env->attack, env->decay, env->sustain, env->release);
    printf("=====================\n");

    return make_custom_oscilator(
        wfid, 
        env->attack,
        env->decay, 
        env->sustain, 
        env->release, 
        spec->octave_increment, 
        spec->coefficient, 
        spec->volume, 
        spec->detune
    );
}

void osc_change_id(struct oscilator *osc, i32 wfid){
    osc->waveform_id = wfid;
}

void osc_update_spec(struct oscilator *osc,  f64 oct, f64 coeff, f64 vol, f64 detune){
    osc->spec = make_spec(oct, coeff, vol, detune);
}

void osc_update_envelope(struct oscilator *osc, f64 atk, f64 dec, f64 sus, f64 rel){
    osc->env = make_env(atk, dec, sus, rel);
}

struct envelope make_env(f64 atk, f64 dec, f64 sus, f64 rel){
    return (struct envelope){
        .state = ENVELOPE_OFF, 
        .envelope = 0.0, 
        .attack = atk, 
        .decay = dec, 
        .sustain = sus, 
        .release = rel
    };
}

struct oscilator make_custom_oscilator(i32 wfid, f64 atk, f64 dec, f64 sus, f64 rel, f64 octpos, f64 coeff, f64 vol, f64 detune){
    return (struct oscilator){ 
        .generated = 0.0,
        .filtered = 0.0,
        .phase = rand_range_f64(0.0, 1.0), 
        .integrator = 0.0, 
        .dcx = 0.0, 
        .dcy = 0.0, 
        .time = 0.0, 
        .waveform_id = wfid, 
        .spec = make_spec(octpos, coeff, vol, detune),
        .env = make_env(atk, dec, sus, rel)
    };
}

struct oscilator make_default_oscilator(i32 wfid){
    return (struct oscilator){ 
        .generated = 0.0,
        .filtered = 0.0,
        .phase = rand_range_f64(0.0, 1.0), 
        .integrator = 0.0, 
        .dcx = 0.0, 
        .dcy = 0.0, 
        .time = 0.0, 
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

struct wave_spec make_spec(f64 oct, f64 coeff, f64 vol, f64 detune){
    return (struct wave_spec) { 
        .octave_increment = oct, 
        .coefficient = coeff, 
        .volume = vol, 
        .detune = detune 
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