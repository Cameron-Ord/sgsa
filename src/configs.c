#include "../include/configs.h"

#include <stdio.h>
#include <string.h>

#include "../include/util.h"
#include "../include/waveform.h"

// https://github-wiki-see.page/m/pret/pokeemerald/wiki/Implementing-ipatix%27s-High-Quality-Audio-Mixer

const f32 DEFAULT_SAMPLERATE = 48000.0f;
const f32 DEFAULT_CHANNELS = 2.0f;

const f32 DEFAULT_VOLUME = 1.0f;
// Don't set gain too high unless you like feedback hell
const f32 DEFAULT_VOICE_GAIN = 2.0f;
const f32 DEFAULT_OSC_GAIN = 8.0f;
const f32 DEFAULT_DELAY_GAIN = 1.5f;
const f32 DEFAULT_DELAY_FEEDBACK = 0.5f;

const f32 DEFAULT_VIBRATO_RATE = 6.0f;
const f32 DEFAULT_VIBRATO_DEPTH = 5.0f;
const f32 DEFAULT_VIBRATO_ON = 0.18f;

void print_config(struct configs config) {
    struct config_entry *ents = config.entries;
    printf("CONFIG = {\n");
    printf("    SAMPLERATE: (%d), CHANNELS: (%d),\n",
           (i32)ents[SAMPLE_RATE_VAL].value, (i32)ents[CHANNELS_VAL].value);
    printf("    VOLUME: (%.3f), SAMPLE GAIN: (%.3f), DELAY "
           "GAIN: (%.3f), "
           "DELAY "
           "FEEDBACK: (%.3f),\n",
           (f64)ents[MAIN_VOLUME_VAL].value, (f64)ents[SAMPLE_GAIN_VAL].value,
           (f64)ents[DELAY_GAIN_VAL].value, (f64)ents[DELAY_FEEDBACK_VAL].value);
    printf("    VRATE: (%.3f), VDEPTH: (%.3f), VIBRATO ON: "
           "(%.3f)\n",
           (f64)ents[VIBRATO_RATE_VAL].value, (f64)ents[VIBRATO_DEPTH_VAL].value,
           (f64)ents[VIBRATO_ONSET_VAL].value);
    printf("}\n");
}

struct configs make_default_config(void) {
    struct configs cfg = {
        .entries = {
            [SAMPLE_RATE_VAL] = { .name = "Sample rate",
                                  .name_len = 0,
                                  .value = DEFAULT_SAMPLERATE },
            [CHANNELS_VAL] = { .name = "Channels",
                               .name_len = 0,
                               .value = DEFAULT_CHANNELS },
            [MAIN_VOLUME_VAL] = { .name = "Main volume",
                                  .name_len = 0,
                                  .value = DEFAULT_VOLUME },
            [SAMPLE_GAIN_VAL] = { .name = "Per voice gain",
                                  .name_len = 0,
                                  .value = DEFAULT_VOICE_GAIN },
            [DELAY_GAIN_VAL] = { .name = "Delay gain",
                                 .name_len = 0,
                                 .value = DEFAULT_DELAY_GAIN },
            [OSC_GAIN_VAL] = { .name = "Per oscilator gain",
                               .name_len = 0,
                               .value = DEFAULT_OSC_GAIN },
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
                                    .value = DEFAULT_VIBRATO_ON },
        }
    };
    for (size_t i = 0; i < CONFIG_END; i++) {
        cfg.entries[i].name_len = strlen(cfg.entries[i].name);
    }
    return cfg;
}