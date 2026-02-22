#ifndef OSCILATOR_H
#define OSCILATOR_H
#include "../include/define.h"
#include "typedef.h"

enum env_locations {
    ENV_STATE_VAL,
    ENVELOPE_VAL,
    ATTACK_VAL,
    DECAY_VAL,
    SUSTAIN_VAL,
    RELEASE_VAL,
    ENV_END,
};

enum spec_locations {
    OCTAVE_VAL,
    COEFF_VAL,
    OSC_VOLUME_VAL,
    DETUNE_VAL,
    SPEC_END,
};

enum state_locations {
    PHASE_VAL,
    INTEGRATOR_VAL,
    DC_X_VAL,
    DC_Y_VAL,
    TIME_VAL,
    STATE_END,
};

struct osc_entry {
    const char *name;
    size_t name_len;
    f32 value;
};

struct wave_spec {
    struct osc_entry entries[SPEC_END];
};

struct envelope {
    struct osc_entry entries[ENV_END];
};

struct osc_state {
    f32 entries[STATE_END];
};

struct osc_generator_values {
    f32 generated[CHANNEL_MAX];
    f32 filtered_high[CHANNEL_MAX];
    f32 filtered_low[CHANNEL_MAX];
};

struct oscilator {
    i32 waveform_id;
    struct osc_generator_values gen;
    struct osc_state state;
    struct wave_spec spec;
    struct envelope env;
};

struct layer {
    u32 oscilators;
    f32 base_freq;
    struct oscilator osc[OSCILATOR_MAX];
};

struct osc_state make_state(void);
struct wave_spec make_spec(f32 oct, f32 coeff, f32 vol, f32 detune);
struct layer make_layer(u32 count, ...);
struct envelope make_env(f32 atk, f32 dec, f32 sus, f32 rel);

struct oscilator make_default_oscilator(i32 wfid);
struct oscilator make_custom_oscilator(i32 wfid, f32 atk, f32 dec, f32 sus,
                                       f32 rel, f32 octpos, f32 coeff, f32 vol,
                                       f32 detune);

void osc_change_id(struct oscilator *osc, i32 wfid);
void osc_update_spec(struct oscilator *osc, f32 oct, f32 coeff, f32 vol,
                     f32 detune);
void osc_update_envelope(struct oscilator *osc, f32 atk, f32 dec, f32 sus,
                         f32 rel);

#endif