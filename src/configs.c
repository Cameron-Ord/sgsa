#include "../include/configs.h"
#include "../include/waveform.h"
#include "../include/util.h"

#include <stdio.h>

const i32 DEFAULT_SAMPLERATE = 44100;
const i32 DEFAULT_CHANNELS = 1;
const i32 DEFAULT_QUANTIZE_DEPTH = 8;

const f32 DEFAULT_VOLUME = 1.0f;
const f32 DEFAULT_SAMPLE_GAIN = 1.0f;
const f32 DEFAULT_DELAY_GAIN = 1.0f;
const f32 DEFAULT_DELAY_FEEDBACK = 1.0f;

const f64 DEFAULT_VIBRATION_RATE = 2.5;
const f64 DEFAULT_VIBRATION_DEPTH = 2.25;
const f64 DEFAULT_VIBRATO_ON = 0.33;


void print_config(struct configs config){
    printf("CONFIG = {\n");
    printf("    SAMPLERATE: (%d), CHANNELS: (%d),\n", config.samplerate, config.channels);
    printf("    VOLUME: (%.3f), SAMPLE GAIN: (%.3f), DELAY GAIN: (%.3f), DELAY FEEDBACK: (%.3f),\n", (f64)config.volume, (f64)config.sample_gain, (f64)config.delay_gain, (f64)config.delay_feedback);
    printf("    VRATE: (%.3f), VDEPTH: (%.3f), VIBRATO ON: (%.3f), QDEPTH: (%d),\n", config.vibration_rate, config.vibration_depth, config.vibrato_on, config.quantize_depth);
    printf("}\n");
}

struct configs make_default_config(void){
    return (struct configs) {
        DEFAULT_SAMPLERATE,
        DEFAULT_CHANNELS,
        DEFAULT_VOLUME,
        DEFAULT_SAMPLE_GAIN,
        DEFAULT_DELAY_GAIN,
        DEFAULT_DELAY_FEEDBACK,
        DEFAULT_VIBRATION_RATE,
        DEFAULT_VIBRATION_DEPTH,
        DEFAULT_VIBRATO_ON,
        DEFAULT_QUANTIZE_DEPTH,
    };
}