#include "../include/configs.h"

#include <stdio.h>
#include <string.h>

#include "../include/util.h"
#include "../include/waveform.h"

// https://github-wiki-see.page/m/pret/pokeemerald/wiki/Implementing-ipatix%27s-High-Quality-Audio-Mixer

const i32 DEFAULT_SAMPLERATE = 48000.0f;
const i32 DEFAULT_CHANNELS = 2.0f;

const f32 DEFAULT_VOLUME = 1.0f;
// Don't set gain too high unless you like feedback hell
const f32 DEFAULT_VOICE_GAIN = 1.25f;
const f32 DEFAULT_DELAY_GAIN = 1.25f;
const f32 DEFAULT_DELAY_FEEDBACK = 0.5f;

const f32 DEFAULT_VIBRATO_RATE = 10.0f;
const f32 DEFAULT_VIBRATO_DEPTH = 0.3f * 0.003f;
const f32 DEFAULT_VIBRATO_ON = 0.18f;

void print_config(struct configs config) {
  struct config_entry_i32 *ivals = config.ivals;
  struct config_entry_f32 *fvals = config.fvals;
  printf("CONFIG = {\n");
  printf("    SAMPLERATE: (%d), CHANNELS: (%d),\n",
         ivals[SAMPLE_RATE_VAL].value, (i32)ivals[CHANNELS_VAL].value);
  printf("    VOLUME: (%.3f), SAMPLE GAIN: (%.3f), DELAY "
         "GAIN: (%.3f), "
         "DELAY "
         "FEEDBACK: (%.3f),\n",
         (f64)fvals[MAIN_VOLUME_VAL].value, (f64)fvals[SAMPLE_GAIN_VAL].value,
         (f64)fvals[DELAY_GAIN_VAL].value,
         (f64)fvals[DELAY_FEEDBACK_VAL].value);
  printf("    VRATE: (%.3f), VDEPTH: (%.3f), VIBRATO ON: "
         "(%.3f)\n",
         (f64)fvals[VIBRATO_RATE_VAL].value,
         (f64)fvals[VIBRATO_DEPTH_VAL].value,
         (f64)fvals[VIBRATO_ONSET_VAL].value);
  printf("}\n");
}

struct configs make_default_config(void) {
  struct configs cfg = {

    { [SAMPLE_RATE_VAL] = { .name = "Sample rate",
                            .name_len = 0,
                            .value = DEFAULT_SAMPLERATE },
      [CHANNELS_VAL] = { .name = "Channels",
                         .name_len = 0,
                         .value = DEFAULT_CHANNELS } },
    { [MAIN_VOLUME_VAL] = { .name = "Main volume",
                            .name_len = 0,
                            .value = DEFAULT_VOLUME },
      [SAMPLE_GAIN_VAL] = { .name = "Per voice gain",
                            .name_len = 0,
                            .value = DEFAULT_VOICE_GAIN },
      [DELAY_GAIN_VAL] = { .name = "Delay gain",
                           .name_len = 0,
                           .value = DEFAULT_DELAY_GAIN },
      [DELAY_FEEDBACK_VAL] = { .name = "Delay feedback",
                               .name_len = 0,
                               .value = DEFAULT_DELAY_FEEDBACK },
      [VIBRATO_RATE_VAL] = { .name = "Vibrato rate",
                             .name_len = 0,
                             .value = DEFAULT_VIBRATO_RATE },
      [VIBRATO_DEPTH_VAL] = { .name = "Vibrato depth",
                              .name_len = 0,
                              .value = DEFAULT_VIBRATO_DEPTH },
      [VIBRATO_ONSET_VAL] = { .name = "Vibrato onset",
                              .name_len = 0,
                              .value = DEFAULT_VIBRATO_ON } }
  };
  for (size_t i = 0; i < CONFIG_INTEGER_VAL_END; i++) {
    cfg.ivals[i].name_len = strlen(cfg.ivals[i].name);
  }

  for (size_t i = 0; i < CONFIG_FLOAT_VAL_END; i++) {
    cfg.fvals[i].name_len = strlen(cfg.fvals[i].name);
  }
  return cfg;
}