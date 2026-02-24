#include <stdarg.h>
#include <string.h>

#include "../include/util.h"
#include "../include/waveform.h"

// Used per oscilator
const f32 DEFAULT_ENV_ATTACK = 0.0f;
const f32 DEFAULT_ENV_DECAY = 0.2f;
const f32 DEFAULT_ENV_SUSTAIN = 0.7f;
const f32 DEFAULT_ENV_RELEASE = 0.0f;
const f32 ZEROED = 0.0f;

const f32 MIN_VAL = 0.0f;
const f32 MAX_VAL = 1.0f;

const f32 DEFAULT_OCTAVE_SKIP = 1.0f;
const f32 DEFAULT_CONTRIBUTION_VOLUME = 1.0f;
const f32 DEFAULT_DETUNE = 1.0f;
const f32 DEFAULT_COEFF = 0.5f;

struct oscilator make_default_oscilator(i32 wfid) {
  struct oscilator osc =
      {.osc_playback_data =
           {[WAVEFORM_ID_VAL] = wfid, [ENVELOPE_STATE_VAL] = ENVELOPE_OFF},
       .gen = {[GEN_ARRAY_RAW] = {ZEROED, ZEROED},
               [GEN_ARRAY_HIGH] = {ZEROED, ZEROED},
               [GEN_ARRAY_LOW] = {ZEROED, ZEROED}},
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
               [ENVELOPE_VAL] = {.name = "Envelope value",
                                 .name_len = strlen("Envelope value"),
                                 .value = ZEROED},
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
       .oscilator_states = {
           [PHASE_VAL] = ZEROED,
           [INTEGRATOR_VAL] = ZEROED,
           [DC_X_VAL] = ZEROED,
           [DC_Y_VAL] = ZEROED,
           [TIME_VAL] = ZEROED,
       }};
  return osc;
}