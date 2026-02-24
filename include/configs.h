#ifndef CONFIGS_H
#define CONFIGS_H
#include "typedef.h"

enum config_locations {
  SAMPLE_RATE_VAL = 0,
  CHANNELS_VAL = 1,
  CONFIG_INTEGER_VAL_END = 2,

  MAIN_VOLUME_VAL = 0,
  SAMPLE_GAIN_VAL = 1,
  DELAY_GAIN_VAL = 2,
  OSC_GAIN_VAL = 3,
  DELAY_FEEDBACK_VAL = 4,
  VIBRATO_RATE_VAL = 5,
  VIBRATO_DEPTH_VAL = 6,
  VIBRATO_ONSET_VAL = 7,
  CONFIG_FLOAT_VAL_END = 8
};

struct config_entry_f32 {
  const char *name;
  size_t name_len;
  f32 value;
};

struct config_entry_i32 {
  const char *name;
  size_t name_len;
  i32 value;
};

struct configs {
  struct config_entry_i32 ivals[CONFIG_INTEGER_VAL_END];
  struct config_entry_f32 fvals[CONFIG_FLOAT_VAL_END];
};

void print_config(struct configs config);
struct configs make_default_config(void);

#endif