#ifndef WAVE_H
#define WAVE_H
#include "configs.h"
#include "define.h"
#include "effect.h"

#include <stdarg.h>
#include <stdbool.h>

enum osc_data_indexes {
  ATTACK_VAL = 0,
  DECAY_VAL = 1,
  SUSTAIN_VAL = 2,
  RELEASE_VAL = 3,
  ENV_END = 4,

  OCTAVE_VAL = 0,
  COEFF_VAL = 1,
  OSC_VOLUME_VAL = 2,
  DETUNE_VAL = 3,
  SPEC_END = 4,

  ENVELOPE_VAL = 0,
  PHASE_VAL = 1,
  INTEGRATOR_VAL = 2,
  DC_X_VAL = 3,
  DC_Y_VAL = 4,
  TIME_VAL = 5,
  STATE_END = 6,

  GEN_ARRAY_RAW = 0,
  GEN_ARRAY_HIGH = 1,
  GEN_ARRAY_LOW = 2,
  GEN_ARRAY_END = 3,
};

struct osc_entry_f32 {
  const char *name;
  size_t name_len;
  f32 value;
};

struct osc_state {
  f32 gen[GEN_ARRAY_END][CHANNEL_MAX];
  f32 oscilator_states[STATE_END];
  i32 envelope_state;
};

struct osc_config {
  i32 waveform_id;
  struct osc_entry_f32 spec[SPEC_END];
  struct osc_entry_f32 env[ENV_END];
};

struct osc_state zeroed_osc_state(void);
struct osc_config default_osc_config(i32 wfid);

struct voice {
  bool active;
  i32 midi_key;
  f32 base_freq;
  f32 amplitude;
  struct osc_state osc[OSCILATOR_MAX];
};

struct layer {
  u32 osc_count;
  f32 dc_blocker;
  bool delay_active;
  struct configs pb_cfg;
  struct delay_line dl;
  struct voice voices[VOICE_MAX];
  struct osc_config osc_cfg;
  f32 layer_window[WINDOW_RESOLUTION];
};

void adsr(f32 *envelope, i32 *state, const f32 *attack, const f32 *decay,
          const f32 *sustain, const f32 *release, i32 samplerate);
f32 vibrato(f32 vrate, f32 depth, f32 freq, i32 samplerate);
f32 tremolo(f32 trate, f32 depth, f32 time);
f32 map_velocity(i32 second);

void voice_set_iterate(struct layer *l, f32 amp, i32 midi_key);
void voice_release_iterate(struct layer *l, i32 midi_key);
void voices_initialize(struct voice voices[VOICE_MAX]);
struct layer make_layer(u32 oscilator_count, bool delay_active,
                        f32 delay_seconds, struct configs cfg);

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