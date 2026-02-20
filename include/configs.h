#ifndef CONFIGS_H
#define CONFIGS_H
#include "typedef.h"

struct configs {
    i32 samplerate;
    i32 channels;

    f32 volume;
    f32 sample_gain;
    f32 delay_gain;
    f32 osc_gain;
    f32 delay_feedback;

    f64 vibration_rate;
    f64 vibration_depth;
    f64 vibrato_on;
    i32 quantize_depth;
};

void print_config(struct configs config);
struct configs make_default_config(void);


#endif