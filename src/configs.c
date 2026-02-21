#include "../include/configs.h"
#include "../include/waveform.h"
#include "../include/util.h"

#include <stdio.h>

//https://github-wiki-see.page/m/pret/pokeemerald/wiki/Implementing-ipatix%27s-High-Quality-Audio-Mixer

const i32 DEFAULT_SAMPLERATE = 48000;
const i32 DEFAULT_CHANNELS = 2;
const i32 DEFAULT_QUANTIZE_DEPTH = 8;

const f32 DEFAULT_VOLUME = 1.0f;
// Don't set gain too high unless you like feedback hell
const f32 DEFAULT_VOICE_GAIN = 16.0f;
const f32 DEFAULT_OSC_GAIN = 8.0f;
const f32 DEFAULT_DELAY_GAIN = 1.5f;
const f32 DEFAULT_DELAY_FEEDBACK = 0.5f;

const f64 DEFAULT_VIBRATION_RATE = 6.0;
const f64 DEFAULT_VIBRATION_DEPTH = 5.0;
const f64 DEFAULT_VIBRATO_ON = 0.18;


void print_config(struct configs config){
    printf("CONFIG = {\n");
    printf("    SAMPLERATE: (%d), CHANNELS: (%d),\n", config.samplerate, config.channels);
    printf("    VOLUME: (%.3f), SAMPLE GAIN: (%.3f), DELAY GAIN: (%.3f), DELAY FEEDBACK: (%.3f),\n", (f64)config.volume, (f64)config.sample_gain, (f64)config.delay_gain, (f64)config.delay_feedback);
    printf("    VRATE: (%.3f), VDEPTH: (%.3f), VIBRATO ON: (%.3f), QDEPTH: (%d),\n", config.vibration_rate, config.vibration_depth, config.vibrato_on, config.quantize_depth);
    printf("}\n");
}

struct configs make_default_config(void){
    return (struct configs) {
        .samplerate = DEFAULT_SAMPLERATE,
        .channels = DEFAULT_CHANNELS,
        .volume = DEFAULT_VOLUME,
        .sample_gain = DEFAULT_VOICE_GAIN,
        .delay_gain = DEFAULT_DELAY_GAIN,
        .osc_gain = DEFAULT_OSC_GAIN,
        .delay_feedback = DEFAULT_DELAY_FEEDBACK,
        .vibration_rate = DEFAULT_VIBRATION_RATE,
        .vibration_depth = DEFAULT_VIBRATION_DEPTH,
        .vibrato_on = DEFAULT_VIBRATO_ON,
        .quantize_depth = DEFAULT_QUANTIZE_DEPTH,
    };
}