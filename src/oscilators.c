#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "../include/oscilator.h"
#include "../include/util.h"

// Used per oscilator
const f32 DEFAULT_ENV_ATTACK = 0.05f;
const f32 DEFAULT_ENV_DECAY = 0.225f;
const f32 DEFAULT_ENV_SUSTAIN = 0.675f;
const f32 DEFAULT_ENV_RELEASE = 0.05f;
const f32 ZEROED = 0.0f;

const f32 MIN_VAL = 0.0f;
const f32 MAX_VAL = 1.0f;

const f32 DEFAULT_OCTAVE_SKIP = 1.0f;
const f32 DEFAULT_CONTRIBUTION_VOLUME = 1.0f;
const f32 DEFAULT_DETUNE = 1.0f;
const f32 DEFAULT_COEFF = 0.33f;

void osc_change_id(struct oscilator *osc, i32 wfid) { osc->waveform_id = wfid; }

void osc_update_spec(struct oscilator *osc, f32 oct, f32 coeff, f32 vol,
                     f32 detune) {
    osc->spec = make_spec(oct, coeff, vol, detune);
}

void osc_update_envelope(struct oscilator *osc, f32 atk, f32 dec, f32 sus,
                         f32 rel) {
    osc->env = make_env(atk, dec, sus, rel);
}

struct osc_state make_state(void) {
    struct osc_state state = {
        .entries = {
            [PHASE_VAL] = ZEROED,
            [INTEGRATOR_VAL] = ZEROED,
            [DC_X_VAL] = ZEROED,
            [DC_Y_VAL] = ZEROED,
            [TIME_VAL] = ZEROED,
        }
    };
    return state;
}

struct envelope make_env(f32 atk, f32 dec, f32 sus, f32 rel) {
    struct envelope env = {
        .entries = {
            [ENV_STATE_VAL] = { .name = "Envelope state",
                                .name_len = 0,
                                .value = ENVELOPE_OFF },
            [ENVELOPE_VAL] = { .name = "Envelope", .name_len = 0, .value = ZEROED },
            [ATTACK_VAL] = { .name = "Attack", .name_len = 0, .value = atk },
            [DECAY_VAL] = { .name = "Decay", .name_len = 0, .value = dec },
            [SUSTAIN_VAL] = { .name = "Sustain", .name_len = 0, .value = sus },
            [RELEASE_VAL] = { .name = "Release", .name_len = 0, .value = rel },
        }
    };
    for (size_t i = 0; i < ENV_END; i++) {
        env.entries[i].name_len = strlen(env.entries[i].name);
    }
    return env;
}

struct oscilator make_custom_oscilator(i32 wfid, f32 atk, f32 dec, f32 sus,
                                       f32 rel, f32 octpos, f32 coeff, f32 vol,
                                       f32 detune) {
    struct oscilator osc = {
        .waveform_id = wfid,
        .gen = {
            .generated = { ZEROED, ZEROED },
            .filtered_high = { ZEROED, ZEROED },
            .filtered_low = { ZEROED, ZEROED },
        },
        .state = make_state(),
        .spec = make_spec(octpos, coeff, vol, detune),
        .env = make_env(atk, dec, sus, rel)
    };
    return osc;
}

struct oscilator make_default_oscilator(i32 wfid) {
    return (struct oscilator){
        .waveform_id = wfid,
        .gen = { { ZEROED, ZEROED }, { ZEROED, ZEROED }, { ZEROED, ZEROED } },
        .state = make_state(),
        .spec = make_spec(DEFAULT_OCTAVE_SKIP, DEFAULT_COEFF,
                          DEFAULT_CONTRIBUTION_VOLUME, DEFAULT_DETUNE),
        .env = make_env(DEFAULT_ENV_ATTACK, DEFAULT_ENV_DECAY, DEFAULT_ENV_SUSTAIN,
                        DEFAULT_ENV_RELEASE)
    };
}

struct wave_spec make_spec(f32 oct, f32 coeff, f32 vol, f32 detune) {
    struct wave_spec spec = {
        .entries = {
            [OCTAVE_VAL] = { "Octave increment", .name_len = 0, .value = oct },
            [COEFF_VAL] = { "Coefficient", .name_len = 0, .value = coeff },
            [OSC_VOLUME_VAL] = { "Oscilator volume", .name_len = 0, .value = vol },
            [DETUNE_VAL] = { "Detune", .name_len = 0, .value = detune },
        }
    };
    for (size_t i = 0; i < SPEC_END; i++) {
        spec.entries[i].name_len = strlen(spec.entries[i].name);
    }
    return spec;
}

struct layer make_layer(u32 count, ...) {
    struct layer l = { 0 };
    l.oscilators = count;
    l.base_freq = ZEROED;
    va_list args;
    va_start(args, count);
    for (u32 i = 0; i < count; i++) {
        l.osc[i] = va_arg(args, struct oscilator);
    }
    va_end(args);
    return l;
}