#include "../include/waveform.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

//Read this and fix your shit, ya dummy
//https://www.martin-finke.de/articles/audio-plugins-018-polyblep-oscillator/

//One of the downsides to using fourier waveforms is that at higher iterations it just
//produces an ungodly amount of harmonics and mix that in with multiple notes
//and you give flubby mess. So just using a hard set constant to limit the harmonic content generated.
//I do wanna add some polyblep versions though
const i32 TRIANGLE_HARMONIC_MAX = 25;
const i32 SAW_HARMONIC_MAX = 50;
const i32 SQUARE_HARMONIC_MAX = 40;

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
        case SQUARE_RAW:{
            return "SQUARE";
        } break;
        case SINE_RAW:{
            return "SINE";
        }break;
        case SAW_RAW:{
            return "SAW";
        }break;
        case TRIANGLE_RAW:{
            return "TRIANGLE";
        }break;
    }
}

f64 vibrato(f64 vrate, f64 depth, f64 freq, f64 samplerate){
    static f64 phase;
    phase += vrate / samplerate; 
    if(phase >= 1.0) phase -= 1.0;

    const f64 mod = sin(2.0 * PI * phase);
    return freq + mod * depth;
}

i32 prev_waveform(const i32 current){
    i32 prev = current - 1;
    if(prev <= WAVE_FORM_BEGIN){
        prev = WAVE_FORM_END - 1;    
    }
    return prev;
}

i32 next_waveform(const i32 current){
    i32 next = current + 1;
    if(next >= WAVE_FORM_END){
        next = WAVE_FORM_BEGIN + 1;    
    }
    return next;
}

// RAW WAVE PRODUCTS

// https://en.wikipedia.org/wiki/Sawtooth_wave
f64 sawtooth(f64 phase){
    return 2.0 * (phase - 0.5);
}

f64 sgn(f64 x, f64 threshold){
    if(x > threshold) return 1.0;
    if(x < threshold) return -1.0;
    return 0.0;
}

f64 square(f64 phase, f64 duty){
    return sgn(cos(2.0 * PI * phase), cos(PI * duty));
}

f64 triangle(f64 phase){
    return 2.0 * fabs(2.0 * (phase - 0.5)) - 1.0;
}

f64 sine(f64 phase){
    return 1.0 * sin(2.0 * PI * phase);
}

void vc_set_waveform(struct voice_control *vc, i32 wfid){
    printf("Set to: %s\n", wfid_to_str(wfid));
    vc->waveform_id = wfid;
}

struct envelope make_env(i32 state, f64 env, f64 release){
    return (struct envelope){state, env, release};
}

struct oscilator make_osciliator(f64 phase, f64 freq){
    return (struct oscilator){phase, freq};
}

struct internal_format make_format(u8 channels, i32 samplerate, u32 format){
    return (struct internal_format){channels, samplerate, format};
}

void vc_set_fmt(struct voice_control *v, struct internal_format fmt){
    struct internal_format *f = &v->fmt;
    f->CHANNELS = fmt.CHANNELS;
    f->FORMAT = fmt.FORMAT;
    f->SAMPLE_RATE = fmt.SAMPLE_RATE;
}

void voice_set_osc(struct voice *v, struct oscilator osc){
    struct oscilator *o = &v->osc;
    o->freq = osc.freq;
    o->phase = osc.phase;
}

void voice_set_env(struct voice *v, struct envelope env){
    struct envelope *e = &v->env;
    e->envelope = env.envelope;
    e->release_increment = env.release_increment;
    e->state = env.state;
}

void vc_initialize(struct voice_control *vc, i32 wfid, struct internal_format fmt, struct oscilator osc, struct envelope env){
    vc->fmt = fmt;
    vc->waveform_id = wfid;
    voices_initialize(vc->voices, osc, env);
}

void voices_initialize(struct voice voices[VOICE_MAX], struct oscilator osc, struct envelope env){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        v->midi_key = -1;
        voice_set_osc(v, osc);
        voice_set_env(v, env);
    }
}

void voice_set_iterate(struct voice voices[VOICE_MAX], i32 midi_key, struct oscilator osc, struct envelope env){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        if(v->env.state == ENVELOPE_OFF){
            v->midi_key = midi_key;
            voice_set_env(v, env);
            voice_set_osc(v, osc);
            return;
        }
    }
}

void voice_release_iterate(struct voice voices[VOICE_MAX], i32 midi_key, i32 samplerate){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        if(v->env.state != ENVELOPE_OFF && v->midi_key == midi_key){
            voice_set_env(v, 
                make_env(ENVELOPE_RELEASE, v->env.envelope, RELEASE_INCREMENT(v->env.envelope, samplerate))
            );
            return;
        }
    }
}

// UNUSED (FOR LEARNING PURPOSES)

//https://en.wikipedia.org/wiki/Pulse_wave
f64 fourier_pulse(f64 phase, f64 duty){
    f64 sum = 0.0;
    for(i32 n = 1; n <= SQUARE_HARMONIC_MAX; n++){
        sum += (1.0 / n) 
        * sin(PI * n * duty) 
        * cos(2.0 * PI * n * phase);
    }
    return duty + (2.0 / PI) * sum;

}

//https://en.wikipedia.org/wiki/Square_wave_(waveform)
f64 fourier_square(f64 phase){
    f64 sum = 0.0;
    for(i32 k = 1; k <= SQUARE_HARMONIC_MAX; k++){
        f64 n = 2.0 * k - 1.0;
        sum += (1.0 / n) * sin(2.0 * PI * n * phase);
    }
    return sum * (4.0 / PI);
}

f64 fourier_sawtooth(f64 phase){
    f64 sum = 0.0;
    for(i32 k = 1; k <= SAW_HARMONIC_MAX; k++){
        sum += pow(-1.0, k) * sin(2.0 * PI * k * phase) / k;
    }
    return sum * -(2 * 1.0 / PI);
}

f64 reverse_fourier_sawtooth(f64 phase){
    f64 sum = 0.0;
    for(i32 k = 1; k <= SAW_HARMONIC_MAX; k++){
        sum += pow(-1.0, k) * sin(2.0 * PI * k * phase) / k;
    }
    return sum * (2 * 1.0 / PI);
}