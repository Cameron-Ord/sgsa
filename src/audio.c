#include "../include/audio.h"
#include "../include/waveform.h"
#include <SDL3/SDL_audio.h>
#include <stdio.h>
#include <math.h>

const f64 VIBRATO_ON = 0.18;
const f32 VOLUME = 1.0f;
const i32 SAMPLE_PER_CALLBACK = 128;
const f32 MASTER_GAIN = 1.5f;
const f64 VRATE = 6.25;
const f64 EFFECT_DEPTH = 5.25;
const i32 BIT_DEPTH = 8;

//const f64 alpha = 1.0 / SAMPLE_RATE;

bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len){
    return SDL_PutAudioStreamData(stream, samples, len);
}

static f64 loop_oscilators(f64 amp, struct layer *l, i32 samplerate, f64 dcblock){
    f64 sum = 0.0;
    for(u32 i = 0; i < l->oscilators; i++){
        f64 generated = 0.0;
        struct oscilator *osc = &l->osc[i];
        f64 freq = l->base_freq * osc->spec.octave_increment * osc->spec.detune;
        if(osc->time > VIBRATO_ON){
            freq = vibrato(VRATE, EFFECT_DEPTH, freq, samplerate);
        }
        const f64 dt = freq / samplerate;

        switch(osc->waveform_id){
            default: break;
            case PULSE_POLY:{
                generated = poly_square(
                    amp, osc->phase, dt, osc->spec.coefficient
                ) * osc->spec.volume;
            }break;
            case TRIANGLE_POLY:{
                generated = poly_triangle(
                    amp, dt, osc->phase,&osc->integrator, &osc->dcx, 
                    &osc->dcy, dcblock
                ) * osc->spec.volume;
            }break;
            case SAW_POLY:{
                generated = poly_saw(
                    amp, dt, osc->phase
                ) * osc->spec.volume;
            }break;
            case SAW_RAW:{
                generated = quantize(sawtooth(
                    amp, osc->phase
                ), 8) * osc->spec.volume;
            }break;
            case TRIANGLE_RAW:{
                generated = quantize(triangle(
                    amp, osc->phase
                ), 8) * osc->spec.volume;
            }break;
            case PULSE_RAW:{
                generated = quantize(square(
                    amp, osc->phase, osc->spec.coefficient
                ), 8) * osc->spec.volume;
            }break;
        }

        osc->phase += dt;
        if(osc->phase >= 1.0) {
            osc->phase -= 1.0;
        }
        osc->time += 1.0 / samplerate;
        sum += generated / l->oscilators;
    }
    return sum;
}

static f32 loop_voicings(struct voice voices[VOICE_MAX], f64 wave_samples[VOICE_MAX], i32 samplerate, f64 dcblock){
    f32 sample = 0.0;
    for(u32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        wave_samples[i] = 0.0;
        const bool cond = (v->active && v->env.state != ENVELOPE_OFF) 
            || (!v->active && v->env.state == ENVELOPE_RELEASE);
        if(cond){
            wave_samples[i] = loop_oscilators(v->amplitude,&v->l, samplerate, dcblock);
            const f64 envelope = adsr(&v->env.state, &v->env.envelope, &v->env.release_increment, samplerate);
            wave_samples[i] *= envelope;
        }
        sample += (f32)tanh(wave_samples[i] * (f64)MASTER_GAIN / VOICE_MAX) * VOLUME;
    }
    return sample;
}

static void loop_samples(size_t count, f32 *samplebuffer, struct voice_control *vc){
    for(size_t n = 0; n < count; n++){
        f64 wave_samples[VOICE_MAX];
        samplebuffer[n] = loop_voicings(vc
            ->voices, wave_samples, 
            vc->fmt.SAMPLE_RATE,
            vc->dcblock
        ) * VOLUME;
    }
}

void stream_callback(void *data, SDL_AudioStream *stream, i32 add, i32 total){
    struct voice_control *vc = (struct voice_control *)data;
    if(!vc) { return; }

    size_t sample_count = (u32)add / sizeof(f32);
    while(sample_count > 0){
        f32 samples[SAMPLE_PER_CALLBACK];
        memset(samples, 0, sizeof(samples));
        const size_t valid_samples = SDL_min(sample_count, SDL_arraysize(samples));
        loop_samples(valid_samples, samples, vc);
        stream_feed(stream, samples, (i32)valid_samples * (i32)sizeof(f32));
        sample_count -= valid_samples;
    }
}

SDL_AudioSpec make_audio_spec(i32 channels, i32 samplerate, SDL_AudioFormat format){
    SDL_AudioSpec spec = {
        format,
        channels,
        samplerate,
    };
    return spec;
}

struct playback_device open_audio_device(void){
    u32 id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if(!id){
        return (struct playback_device){ 0, NULL, { 0, 0, 0 }, false };
    }
    SDL_AudioSpec obtained = {0};
    SDL_GetAudioDeviceFormat(id, &obtained, NULL);
    const char *name = SDL_GetAudioDeviceName(id);
    printf("Device name: %s, Output spec: 0x%X - %d - %d\n", name, obtained.format, obtained.channels, obtained.freq);
    return (struct playback_device){ id, NULL, obtained, true };
}

SDL_AudioStream *audio_stream_create(SDL_AudioSpec internal, SDL_AudioSpec device_out){
    SDL_AudioStream *stream = SDL_CreateAudioStream(&internal, &device_out);
    if(!stream){
        printf("%s\n", SDL_GetError());
        return NULL;
    }
    return stream;
}

bool set_audio_callback(SDL_AudioStream *stream, struct voice_control *data){
    if(!SDL_SetAudioStreamGetCallback(stream, stream_callback, data)){
        printf("%s\n", SDL_GetError());
        return false;
    }
    return true;
}

void audio_stream_unbind(SDL_AudioStream *stream){
    if(stream){
        SDL_UnbindAudioStream(stream);
    }
}

bool audio_stream_bind(SDL_AudioStream *stream, u32 id){
    if(stream && id){
        if(!SDL_BindAudioStream(id, stream)){
            printf("%s\n", SDL_GetError());
            return false;
        }
        return true;
    }
    return false;
}

void resume_audio(u32 id){
    if(id){
        SDL_ResumeAudioDevice(id);
    }
}

void pause_audio(u32 id){
    if(id){
        SDL_PauseAudioDevice(id);
    }
}

void close_audio_device(u32 id){
    if(id){
        SDL_CloseAudioDevice(id);
    }
}

void audio_stream_destroy(SDL_AudioStream *stream){
    if(stream){
        SDL_DestroyAudioStream(stream);
    }
}