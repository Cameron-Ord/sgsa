#include <stdarg.h>
#include <string.h>

#include "../include/util.h"
#include "../include/waveform.h"

// Used per oscilator
const f32 DEFAULT_ENV_ATTACK = 0.4f;
const f32 DEFAULT_ENV_DECAY = 0.75f;
const f32 DEFAULT_ENV_SUSTAIN = 0.8f;
const f32 DEFAULT_ENV_RELEASE = 0.4f;
const f32 ZEROED = 0.0f;

const f32 MIN_VAL = 0.0f;
const f32 MAX_VAL = 1.0f;

const f32 DEFAULT_OCTAVE_SKIP = 1.0f;
const f32 DEFAULT_CONTRIBUTION_VOLUME = 1.0f;
const f32 DEFAULT_DETUNE = 1.0f;
const f32 DEFAULT_COEFF = 0.5f;

struct osc_state zeroed_osc_state(i32 preset_state, f32 preset_phase) {
  struct osc_state osc =
      {
        .gen = {[GEN_ARRAY_RAW] = {ZEROED, ZEROED},
               [GEN_ARRAY_HIGH] = {ZEROED, ZEROED},
               [GEN_ARRAY_LOW] = {ZEROED, ZEROED}},

       .oscilator_states = {
           [ENVELOPE_VAL] = ZEROED,
           [PHASE_VAL] = preset_phase,
           [MOD_PHASE_VAL] = ZEROED,
           [INTEGRATOR_VAL] = ZEROED,
           [DC_X_VAL] = ZEROED,
           [DC_Y_VAL] = ZEROED,
           [TIME_VAL] = ZEROED,
       },
       .envelope_state = preset_state    
    };
  return osc;
}

struct osc_config default_osc_config(i32 wfid) {
  struct osc_config osc = {
        .waveform_id = wfid,
               .spec = {[OCTAVE_VAL] = {.name = "Octave increment",
                                .name_len = strlen("Octave increment"),
                                .value = DEFAULT_OCTAVE_SKIP},
                [COEFF_VAL] = {.name = "Coefficient value",
                               .name_len = strlen("Coefficient value"),
                               .value = DEFAULT_COEFF},
                [OSC_VOLUME_VAL] = {.name = "Oscilator volume",
                                    .name_len = strlen("Oscilator volume"),
                                    .value = DEFAULT_CONTRIBUTION_VOLUME},
                [DETUNE_VAL] = {.name = "Detune value",
                                .name_len = strlen("Detune value"),
                                .value = DEFAULT_DETUNE}},
       .env =
           {
               [ATTACK_VAL] = {.name = "Attack value",
                               .name_len = strlen("Attack value"),
                               .value = DEFAULT_ENV_ATTACK},
               [DECAY_VAL] = {.name = "Decay value",
                              .name_len = strlen("Decay value"),
                              .value = DEFAULT_ENV_DECAY},
               [SUSTAIN_VAL] = {.name = "Sustain value",
                                .name_len = strlen("Sustain value"),
                                .value = DEFAULT_ENV_SUSTAIN},
               [RELEASE_VAL] = {.name = "Release value",
                                .name_len = strlen("Release value"),
                                .value = DEFAULT_ENV_RELEASE},
           },
    };
  return osc;
}