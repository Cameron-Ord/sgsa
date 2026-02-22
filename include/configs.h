#ifndef CONFIGS_H
#define CONFIGS_H
#include "typedef.h"

enum config_locations {
    SAMPLE_RATE,
    CHANNELS,
    MAIN_VOLUME,
    SAMPLE_GAIN,
    DELAY_GAIN,
    OSC_GAIN,
    DELAY_FEEDBACK,
    VIBRATO_RATE,
    VIBRATO_DEPTH,
    VIBRATO_ONSET,
    CONFIG_END
};

struct config_entry {
    const char* name;
    size_t name_len;
    f32 value;
};

struct configs {
    struct config_entry entries[CONFIG_END];
};

void print_config(struct configs config);
struct configs make_default_config(void);

#endif