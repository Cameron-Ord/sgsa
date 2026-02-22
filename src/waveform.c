#include "../include/waveform.h"
#include "../include/configs.h"
#include "../include/effect.h"
#include "../include/util.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
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
f32 poly_triangle(f32 amp, f32 inc, f32 phase, f32* integrator, f32* x, f32* y, f32 block) {
    f32 sqr = poly_square(1.0f, inc, phase, 0.5f);
    sqr *= inc;
    *integrator += sqr;
    *y = (*integrator * 4.0f) - *x + block * *y;
    *x = (*integrator * 4.0f);
    return *y * amp;
}

f32 poly_saw(f32 amp, f32 inc, f32 phase) {
    f32 saw = sawtooth(1.0, phase);
    saw -= polyblep(inc, phase);
    return amp * saw;
}

// https://en.wikipedia.org/wiki/Sawtooth_wave
f32 sawtooth(f32 amp, f32 phase) { return amp * (2.0f * phase - 1.0f); }

f32 square(f32 amp, f32 phase, f32 duty) { return amp * ((phase < duty) ? 1.0f : -1.0f); }

f32 triangle(f32 amp, f32 phase) { return amp * (2.0f * fabsf(2.0f * (phase - 0.5f)) - 1.0f); }

f32 sine(f32 amp, f32 phase) { return amp * (1.0f * sinf(2.0f * PI * phase)); }

f32 vibrato(f32 vrate, f32 depth, f32 freq, f32 samplerate) {
    static f32 phase;
    phase += vrate / samplerate;
    if (phase >= 1.0f)
        phase -= 1.0f;

    const f32 mod = sinf(2.0f * PI * phase);
    return freq + mod * depth;
}

f32 tremolo(f32 trate, f32 depth, f32 phase) { return 1.0f + depth * sinf(2.0f * PI * trate * phase); }

void adsr(f32* envelope, f32* state, const f32* attack, const f32* decay, const f32* sustain, const f32* release,
          i32 samplerate) {
    switch ((i32)*state) {
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

static char* wfid_to_str(i32 wfid) {
    switch (wfid) {
    default:
        return "Unknown ID";
    case SAW_POLY: {
        return "Polyblep Sawtooth";
    } break;
    case PULSE_POLY: {
        return "Polyblep Square";
    } break;
    case TRIANGLE_POLY: {
        return "Polyblep Triangle";
    } break;

    case SAW_RAW: {
        return "Sawtooth";
    } break;
    case PULSE_RAW: {
        return "Square";
    } break;
    case TRIANGLE_RAW: {
        return "Triangle";
    } break;
    }
}

u32 prev_layer(u32 current, u32 last) {
    i32 scurrent = (i32)current;
    const i32 slast = (i32)last;
    i32 prev = scurrent - 1;
    if (prev < 0) {
        prev = slast - 1;
    }
    return (u32)prev;
}

u32 next_layer(u32 current, u32 last) {
    u32 next = current + 1;
    if (next >= last) {
        next = 0;
    }
    return next;
}

f32 map_velocity(i32 second) {
    f32 base_amp = 1.0f, low_scale = 0.0125f, high_scale = 0.0225f;
    const i32 high_threshold = 75;
    const i32 low_threshold = 45;

    if (second > high_threshold) {
        base_amp += (f32)(second - high_threshold) * high_scale;
    } else if (second < low_threshold) {
        base_amp -= (f32)(low_threshold - second) * low_scale;
    }

    if (base_amp < 0.25f) {
        base_amp = 0.125f;
    } else if (base_amp > 2.25f) {
        base_amp = 2.25f;
    }

    return base_amp;
}

void vc_initialize(struct voice_control* vc) {
    vc->render_buffer = NULL;
    vc->rbuflen = 0;
    vc->cfg = make_default_config();
    vc->dl = create_delay_line(MS_BUFSIZE(vc->cfg.entries[SAMPLE_RATE].value, 1.5f));
    voices_initialize(vc->voices);
    vc->dcblock = expf(-1.0f / (0.0025f * vc->cfg.entries[SAMPLE_RATE].value));
}

void voices_initialize(struct voice voices[VOICE_MAX]) {
    for (i32 i = 0; i < VOICE_MAX; i++) {
        struct voice* v = &voices[i];
        v->midi_key = -1;
        v->amplitude = 1.0f;
        v->active = false;
        v->l = make_layer(4, make_custom_oscilator(SAW_POLY, 0.1f, 0.1f, 0.2f, 0.2f, 1.0f, 0.5f, 1.0f, 1.0f),
                          make_custom_oscilator(SAW_POLY, 0.1f, 0.1f, 0.2f, 0.2f, 1.5f, 0.5f, 1.0f, 1.0f),
                          make_custom_oscilator(SAW_POLY, 0.1f, 0.1f, 0.2f, 0.2f, 1.0f, 0.5f, 0.8f, 0.996f),
                          make_custom_oscilator(SINE, 0.1f, 0.1f, 0.2f, 0.2f, 0.5f, 0.5f, 0.6f, 1.0f));
    }
}

void layers_set_adsr(struct voice voices[VOICE_MAX], f32 atk, f32 dec, f32 sus, f32 rel) {
    for (i32 i = 0; i < VOICE_MAX; i++) {
        struct voice* v = &voices[i];
        for (size_t k = 0; k < v->l.oscilators; k++) {
            v->l.osc[k].env.entries[ATTACK].value = atk;
            v->l.osc[k].env.entries[DECAY].value = dec;
            v->l.osc[k].env.entries[SUSTAIN].value = sus;
            v->l.osc[k].env.entries[RELEASE].value = rel;
        }
    }
}

void voice_set_iterate(struct voice voices[VOICE_MAX], f32 amp, i32 midi_key) {
    for (i32 i = 0; i < VOICE_MAX; i++) {
        struct voice* v = &voices[i];
        if (!v->active) {
            v->midi_key = midi_key;
            v->amplitude = amp;
            v->active = true;
            v->l.base_freq = midi_to_base_freq(midi_key);
            for (size_t k = 0; k < v->l.oscilators; k++) {
                for (i32 c = 0; c < CHANNEL_MAX; c++) {
                    v->l.osc[k].gen.generated[c] = 0.0f;
                    v->l.osc[k].gen.filtered_high[c] = 0.0f;
                    v->l.osc[k].gen.filtered_low[c] = 0.0f;
                }
                v->l.osc[k].state.entries[PHASE] = rand_range_f32(0.0f, 0.75f);
                v->l.osc[k].env.entries[STATE].value = (f32)ENVELOPE_ATTACK;
            }
            return;
        }
    }
}

void voice_release_iterate(struct voice voices[VOICE_MAX], i32 midi_key) {
    for (i32 i = 0; i < VOICE_MAX; i++) {
        struct voice* v = &voices[i];
        if (v->active && v->midi_key == midi_key) {
            for (size_t k = 0; k < v->l.oscilators; k++) {
                v->l.osc[k].env.entries[STATE].value = (f32)ENVELOPE_RELEASE;
            }

            v->active = false;
            return;
        }
    }
}

void vc_assign_render_buffer(struct voice_control* vc, f32* buffer, size_t len) {
    vc->render_buffer = buffer;
    vc->rbuflen = len;
}