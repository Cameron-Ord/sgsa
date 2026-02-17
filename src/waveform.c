#include "../include/waveform.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

f64 rand_f64(void){
    return (f64)rand() / (f64)RAND_MAX;
}

f64 rand_range_f64(f64 x, f64 y){
    return x + (y - x) * rand_f64();
}

f64 quantize(f64 x, i32 depth){
    const i32 step = (i32)round(x * depth);
    return (f64)step / depth;
}
//polybleppers
f64 polyblep(f64 dt, f64 phase){
    if(phase < dt){
        phase /= dt;
        return phase + phase - phase * phase - 1.0;
    } else if (phase > 1.0 - dt){
        phase = (phase - 1.0) / dt;
        return phase * phase + phase + phase + 1.0;
    }
    return 0.0;
}

f64 poly_square(f64 amp, f64 dt, f64 phase, f64 duty){
    f64 sqr = square(1.0, phase, duty);
    sqr += polyblep(dt, phase);
    sqr -= polyblep(dt, fmod(phase + duty, 1.0));
    return amp * sqr;
} 

// I am really starting to hate triangles
// https://pbat.ch/sndkit/blep/
f64 poly_triangle(f64 amp, f64 dt, f64 phase, f64 freq, f64 *integrator, f64 *x, f64 *y, f64 block){
    f64 sqr = poly_square(1.0, dt, phase, 0.5);
    sqr *= dt;
    *integrator += sqr;
    *y = (*integrator * 4.0) - *x + block * *y;
    *x = (*integrator * 4.0);
    return *y * amp;
}

f64 poly_saw(f64 amp, f64 dt, f64 phase){
    f64 saw = sawtooth(1.0, phase);
    saw -= polyblep(dt, phase);
    return amp * saw;
}

// https://en.wikipedia.org/wiki/Sawtooth_wave
f64 sawtooth(f64 amp, f64 phase){
    return amp * (2.0 * (phase - 1.0));
}

f64 square(f64 amp, f64 phase, f64 duty){
    return amp * phase < duty ? 1.0 : -1.0;
}

f64 triangle(f64 amp, f64 phase){
    return amp * (2.0 * fabs(2.0 * (phase - 0.5)) - 1.0);
}

f64 sine(f64 amp, f64 phase){
    return amp * (1.0 * sin(2.0 * PI * phase));
}

f64 vibrato(f64 vrate, f64 depth, f64 freq, f64 samplerate){
    static f64 phase;
    phase += vrate / samplerate; 
    if(phase >= 1.0) phase -= 1.0;

    const f64 mod = sin(2.0 * PI * phase);
    return freq + mod * depth;
}

f64 tremolo(f64 trate, f64 depth, f64 phase){
    return 1.0 + depth * sin(2.0 * PI * trate * phase);
}

f64 adsr(i32 *state, f64 *envelope, const f64 *release, i32 samplerate){
    f64 mutated = *envelope;
    switch(*state){
        default:break;
        case ENVELOPE_ATTACK:{
            mutated += ATTACK_INCREMENT(samplerate);
            if(mutated >= 1.0){
                mutated = 1.0;
                *state = ENVELOPE_DECAY;
            }
        }break;
    
        case ENVELOPE_SUSTAIN: break;

        case ENVELOPE_DECAY: {
            mutated -= DECAY_INCREMENT(samplerate);
            if(mutated <= SUSTAIN_LEVEL){
                mutated = SUSTAIN_LEVEL;
                *state = ENVELOPE_SUSTAIN;
            }
        }break;

        case ENVELOPE_RELEASE:{
            mutated -= *release;
            if(mutated <= 0.0){
                mutated = 0.0;
                *state = ENVELOPE_OFF;
            }
        }break;

        case ENVELOPE_OFF: break;
    }
    *envelope = mutated;
    return mutated;
}


static char *wfid_to_str(i32 wfid){
    switch(wfid){
        default: return "Unknown ID";
        case SAW_RAW:{
            return "Sawtooth";
        }break;
        case PULSE_RAW:{
            return "Square";
        }break;
        case TRIANGLE_RAW:{
            return "Triangle";
        }break;
    }
}

void print_layer(const char *msg, struct layer l){
    printf("=======\n");
    for(u32 i = 0; i < l.oscilators; i++){
        struct wave_spec s = l.osc[i].spec;
        printf("%s->%d: %s {OCTAVE: %.3f, COEFF: %.3f, VOLUME: %.3f DETUNE: %.3f}\n", 
            msg,
            i + 1, 
            wfid_to_str(l.osc[i].waveform_id), 
            s.octave_increment, s.coefficient, s.volume, s.detune
        );
    }
    printf("=======\n");
}

u32 prev_layer(u32 current, u32 last){
    i32 scurrent = (i32)current;
    const i32 slast = (i32)last; 
    i32 prev = scurrent - 1; 
    if(prev < 0){
        prev = slast - 1;
    }
    return (u32)prev;
}

u32 next_layer(u32 current, u32 last){
    u32 next = current + 1;
    if(next >= last){
        next = 0;    
    }
    return next;
}

f64 map_velocity(i32 second){
    f64 base_amp = 1.0, low_scale = 0.0125, high_scale = 0.0225;
    const i32 high_threshold = 75;
    const i32 low_threshold = 45;
    
    if(second > high_threshold){
        base_amp += (second - high_threshold) * high_scale;
    } else if (second < low_threshold){
        base_amp -= (low_threshold - second) * low_scale;
    }

    if(base_amp < 0.25){
        base_amp = 0.125;
    } else if (base_amp > 2.25){
        base_amp = 2.25;
    }

    return base_amp;
}

struct wave_spec make_wave_spec(f64 octave_increment, f64 coefficient, f64 volume, f64 detune){
    return (struct wave_spec) { octave_increment, coefficient, volume, 1.0 + detune };
}

struct layer make_layer(u32 count, ...){
    struct layer l = {0};
    l.oscilators = count;
    l.base_freq = 0.0;
    va_list args;
    va_start(args, count);
    for(u32 i = 0; i < count; i++){
        l.osc[i] = va_arg(args, struct oscilator);
    }
    va_end(args);
    print_layer("Created layer", l);
    return l;
}

struct envelope make_env(i32 state, f64 env, f64 release){
    return (struct envelope){state, env, release};
}

struct oscilator make_oscilator(i32 wfid, struct wave_spec spec){
    return (struct oscilator){ 
        .phase = rand_range_f64(0.0, 1.0), 
        .integrator = 0.0, 
        .dcx = 0.0, 
        .dcy = 0.0, 
        .time = 0.0, 
        .waveform_id = wfid, 
        .spec = spec
    };
}

struct internal_format make_format(u8 channels, i32 samplerate, u32 format){
    return (struct internal_format){ channels, samplerate, format };
}

struct layer set_layer_freq(struct layer l, f64 freq){
    l.base_freq = freq;
    return l;
}

void vc_set_fmt(struct voice_control *v, struct internal_format fmt){
    v->fmt.CHANNELS = fmt.CHANNELS;
    v->fmt.FORMAT = fmt.FORMAT;
    v->fmt.SAMPLE_RATE = fmt.SAMPLE_RATE;
}

void voice_set_layer(struct voice *v, struct layer l){
    v->l = l;
}

void voice_set_env(struct voice *v, struct envelope env){
    v->env = env;
}

void vc_initialize(struct voice_control *vc, struct internal_format fmt, struct layer l, struct envelope env){
    vc->fmt = fmt;
    voices_initialize(vc->voices, l, env);
    vc->dcblock = exp(-1.0/(0.0025 * fmt.SAMPLE_RATE));
    printf("%.3f\n", vc->dcblock);
}

void voices_initialize(struct voice voices[VOICE_MAX], struct layer l, struct envelope env){
    print_layer("Initializing voices with layer", l);
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        v->midi_key = -1;
        v->amplitude = 1.0;
        v->active = false;
        voice_set_layer(v, l);
        voice_set_env(v, env);
    }
}

void voice_set_iterate(struct voice voices[VOICE_MAX], f64 amp, i32 midi_key, struct layer l, struct envelope env){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        if(v->env.state == ENVELOPE_OFF && !v->active){
            v->midi_key = midi_key;
            v->amplitude = amp;
            voice_set_layer(v, l);
            voice_set_env(v, env);
            v->active = true;
            return;
        }
    }
}

void voice_release_iterate(struct voice voices[VOICE_MAX], i32 midi_key, i32 samplerate){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        if(v->active && v->midi_key == midi_key){
            if(v->env.state != ENVELOPE_OFF){
                voice_set_env(v, 
                    make_env(ENVELOPE_RELEASE, v->env.envelope, RELEASE_INCREMENT(v->env.envelope, samplerate))
                );
                v->active = false;
                return;
            }
        }
    }
}