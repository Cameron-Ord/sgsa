#include "../include/waveform.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "../include/configs.h"
#include "../include/effect.h"
#include "../include/util.h"
// polybleppers
f32 polyblep(f32 inc, f32 phase) {
  if (phase < inc) {
    phase /= inc;
    return phase + phase - phase * phase - 1.0f;
  } else if (phase > 1.0f - inc) {
    phase = (phase - 1.0f) / inc;
    return phase * phase + phase + phase + 1.0f;
  }
  return 0.0;
}

f32 poly_square(f32 amp, f32 inc, f32 phase, f32 duty) {
  f32 sqr = square(1.0f, phase, duty);
  sqr += polyblep(inc, phase);
  sqr -= polyblep(inc, fmodf(phase + duty, 1.0f));
  return amp * sqr;
}
// I am really starting to hate triangles
// https://pbat.ch/sndkit/blep/
f32 poly_triangle(f32 amp, f32 inc, f32 phase, f32 *integrator, f32 *x, f32 *y,
                  f32 block) {
  f32 sqr = poly_square(1.0f, inc, phase, 0.5f);
  sqr *= inc;
  *integrator += sqr;
  *y = (*integrator * 4.0f) - *x + block * *y;
  *x = (*integrator * 4.0f);
  return *y * amp;
}

f32 poly_saw(f32 amp, f32 inc, f32 phase) {
  f32 saw = sawtooth(1.0f, phase);
  saw -= polyblep(inc, phase);
  return amp * saw;
}

// https://en.wikipedia.org/wiki/Sawtooth_wave
f32 sawtooth(f32 amp, f32 phase) { return amp * (2.0f * phase - 1.0f); }

f32 square(f32 amp, f32 phase, f32 duty) {
  return amp * ((phase < duty) ? 1.0f : -1.0f);
}

f32 triangle(f32 amp, f32 phase) {
  return amp * (2.0f * fabsf(2.0f * (phase - 0.5f)) - 1.0f);
}

f32 sine(f32 amp, f32 phase) { return amp * sinf(2.0f * PI * phase); }

f32 vibrato(f32 depth, f32 mod_phase) {
  return depth * sinf(2.0f * PI * mod_phase);
}

f32 tremolo(f32 trate, f32 depth, f32 phase) {
  return 1.0f + depth * sinf(2.0f * PI * trate * phase);
}

void adsr(f32 *envelope, i32 *state, const f32 *attack, const f32 *decay,
          const f32 *sustain, const f32 *release, i32 samplerate) {
  switch (*state) {
  default:
    break;
  case ENVELOPE_ATTACK: {
    *envelope += ATTACK_INCREMENT((f32)samplerate, *attack);
    if (*envelope >= 1.0f) {
      *envelope = 1.0f;
      *state = ENVELOPE_DECAY;
    }
  } break;

  case ENVELOPE_SUSTAIN:
    break;

  case ENVELOPE_DECAY: {
    *envelope -= DECAY_INCREMENT((f32)samplerate, *decay, *sustain);
    if (*envelope <= *sustain) {
      *envelope = *sustain;
      *state = ENVELOPE_SUSTAIN;
    }
  } break;

  case ENVELOPE_RELEASE: {
    *envelope -= RELEASE_INCREMENT(*envelope, (f32)samplerate, *release);
    if (*envelope <= 0.0f) {
      *envelope = 0.0f;
      *state = ENVELOPE_OFF;
    }
  } break;

  case ENVELOPE_OFF:
    break;
  }
}

struct layer make_layer(u32 oscilator_count, bool delay_active,
                        f32 delay_seconds, struct configs cfg) {
  struct layer voice_layer = {
    .osc_count = oscilator_count,
    .dc_blocker =
     expf(-1.0f / (0.0025f * (f32)cfg.ivals[SAMPLE_RATE_VAL].value)),
    .delay_active = delay_active,
    .pb_cfg = cfg,
    .dl = create_delay_line(
     MS_BUFSIZE((f32)cfg.ivals[SAMPLE_RATE_VAL].value, delay_seconds)),
    .osc_cfg = default_osc_config(SAW_POLY)
  };
  memset(voice_layer.layer_window, 0, sizeof(f32) * WINDOW_RESOLUTION);
  voices_initialize(voice_layer.voices);
  return voice_layer;
}

void voices_initialize(struct voice voices[VOICE_MAX]) {
  for (i32 i = 0; i < VOICE_MAX; i++) {
    voices[i].midi_key = -1;
    voices[i].amplitude = 1.0f;
    voices[i].active = false;

    for (u32 k = 0; k < OSCILATOR_MAX; k++) {
      voices[i].osc[k] = zeroed_osc_state(ENVELOPE_OFF, 0.0f);
    }
  }
}

void voice_set_iterate(struct layer *l, f32 amp, i32 midi_key) {
  for (i32 i = 0; i < VOICE_MAX; i++) {
    if (!l->voices[i].active) {
      l->voices[i].midi_key = midi_key;
      l->voices[i].amplitude = amp;
      l->voices[i].active = true;
      l->voices[i].base_freq = midi_to_base_freq(midi_key);

      for (u32 k = 0; k < l->osc_count; k++) {
        for (i32 c = 0; c < CHANNEL_MAX; c++) {
          l->voices[i].osc[k].gen[GEN_ARRAY_RAW][c] = 0.0f;
          l->voices[i].osc[k].gen[GEN_ARRAY_HIGH][c] = 0.0f;
          l->voices[i].osc[k].gen[GEN_ARRAY_LOW][c] = 0.0f;
        }
        l->voices[i].osc[k] =
         zeroed_osc_state(ENVELOPE_ATTACK, rand_range_f32(0.0f, 0.5f));
      }
      return;
    }
  }
}

void voice_release_iterate(struct layer *l, i32 midi_key) {
  for (i32 i = 0; i < VOICE_MAX; i++) {
    if (l->voices[i].active && l->voices[i].midi_key == midi_key) {
      for (u32 k = 0; k < l->osc_count; k++) {
        l->voices[i].osc[k].envelope_state = ENVELOPE_RELEASE;
      }

      l->voices[i].active = false;
      return;
    }
  }
}