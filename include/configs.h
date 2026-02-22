#ifndef CONFIGS_H
#define CONFIGS_H
#include "typedef.h"

enum config_locations {
    SAMPLE_RATE_VAL,
    CHANNELS_VAL,
    MAIN_VOLUME_VAL,
    SAMPLE_GAIN_VAL,
    DELAY_GAIN_VAL,
    OSC_GAIN_VAL,
    DELAY_FEEDBACK_VAL,
    VIBRATO_RATE_VAL,
    VIBRATO_DEPTH_VAL,
    VIBRATO_ONSET_VAL,
    CONFIG_END
};

struct config_entry {
    const char *name;
    size_t name_len;
    f32 value;
};

struct configs {
    struct config_entry entries[CONFIG_END];
};

void print_config(struct configs config);
struct configs make_default_config(void);

#endif