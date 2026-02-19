#include "../include/waveform.h"
#include "../include/util.h"
#include "../include/configs.h"
#include "../include/effect.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
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
f64 poly_triangle(f64 amp, f64 dt, f64 phase, f64 *integrator, f64 *x, f64 *y, f64 block){
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
    return amp * (2.0 * phase - 1.0);
}

f64 square(f64 amp, f64 phase, f64 duty){
    return amp * ((phase < duty) ? 1.0 : -1.0);
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

void adsr(struct envelope *env, i32 samplerate){
    switch(env->state){
        default:break;
        case ENVELOPE_ATTACK:{
            env->envelope += ATTACK_INCREMENT(samplerate, env->attack);
            if(env->envelope >= 1.0){
                env->envelope = 1.0;
                env->state = ENVELOPE_DECAY;
            }
        }break;
    
        case ENVELOPE_SUSTAIN: break;

        case ENVELOPE_DECAY: {
            env->envelope -= DECAY_INCREMENT(samplerate, env->decay, env->sustain);
            if(env->envelope <= env->sustain){
                env->envelope = env->sustain;
                env->state = ENVELOPE_SUSTAIN;
            }
        }break;

        case ENVELOPE_RELEASE:{
            env->envelope -= RELEASE_INCREMENT(env->envelope, samplerate, env->release);
            if(env->envelope <= 0.0){
                env->envelope = 0.0;
                env->state = ENVELOPE_OFF;
            }
        }break;

        case ENVELOPE_OFF: break;
    }
}


static char *wfid_to_str(i32 wfid){
    switch(wfid){
        default: return "Unknown ID";
        case SAW_POLY:{
            return "Polyblep Sawtooth";
        }break;
        case PULSE_POLY:{
            return "Polyblep Square";
        }break;
        case TRIANGLE_POLY:{
            return "Polyblep Triangle";
        }break;

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

void vc_initialize(struct voice_control *vc){
    vc->render_buffer = NULL;
    vc->rbuflen = 0;
    vc->cfg = make_default_config();
    vc->dl = create_delay_line(MS_BUFSIZE(vc->cfg.samplerate, 0.5));

    voices_initialize(vc->voices);
    vc->dcblock = exp(-1.0/(0.0025 * vc->cfg.samplerate));
}

void voices_initialize(struct voice voices[VOICE_MAX]){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        v->midi_key = -1;
        v->amplitude = 1.0;
        v->active = false;
        v->l = make_layer(1, make_square());
    }
}

void layers_set_adsr(struct voice voices[VOICE_MAX], f64 atk, f64 dec, f64 sus, f64 rel){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        for(size_t k = 0; k < v->l.oscilators; k++){
            v->l.osc[k].env.attack = atk;
            v->l.osc[k].env.decay = dec;
            v->l.osc[k].env.sustain = sus;
            v->l.osc[k].env.release = rel;
        }
    }
}

void voice_set_iterate(struct voice voices[VOICE_MAX], f64 amp, i32 midi_key){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        if(!v->active){
            v->midi_key = midi_key;
            v->amplitude = amp;
            v->active = true;
            v->l.base_freq = midi_to_base_freq(midi_key);
            for(size_t k = 0; k < v->l.oscilators; k++){
                v->l.osc[k].env.state = ENVELOPE_ATTACK;
            }
            return;
        }
    }
}

void voice_release_iterate(struct voice voices[VOICE_MAX], i32 midi_key){
    for(i32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        if(v->active && v->midi_key == midi_key){
            for(size_t k = 0; k < v->l.oscilators; k++){
                v->l.osc[k].env.state = ENVELOPE_RELEASE;
            }
            v->active = false;
            return;
        }
    }
}

void vc_assign_render_buffer(struct voice_control *vc, f32 *buffer, size_t len){
    vc->render_buffer = buffer;
    vc->rbuflen = len;
}