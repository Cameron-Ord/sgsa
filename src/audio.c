#include "../include/audio.h"
#include "../include/waveform.h"
#include "../include/effect.h"

#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include <stdio.h>
#include <math.h>

const size_t SAMPLE_PER_CALLBACK = 128;

static void render_push(f32* samples, size_t nsamples, f32 *buffer, size_t buflen){
    if (nsamples > 0 && samples) {
        memmove(buffer, buffer + nsamples, (buflen - nsamples) * sizeof(f32));
        memcpy(buffer + (buflen - nsamples), samples, nsamples * sizeof(f32));
    } 
}

bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len){
    return SDL_PutAudioStreamData(stream, samples, len);
}

static void loop_delay(size_t nsamples, f32 *samples, struct voice_control *vc){
    for(size_t i = 0; i < nsamples; i++){
        const f32 delayed = delay_line_read(&vc->dl) * vc->cfg.delay_feedback;
        const f32 mixed = samples[i] + (0.5f * delayed);
        samples[i] = tanhf(mixed * vc->cfg.delay_gain);
        delay_line_write(samples[i], &vc->dl);
    }
}

static f64 loop_oscilators(struct voice *v, const struct configs *cfg, f64 dcblock){
    f64 sum = 0.0;
    for(u32 i = 0; i < v->l.oscilators; i++){
        f64 generated = 0.0;
        struct oscilator *osc = &v->l.osc[i];
        f64 freq = v->l.base_freq * osc->spec.octave_increment * osc->spec.detune;
        if(osc->time > cfg->vibrato_on){
            freq = vibrato(cfg->vibration_rate, cfg->vibration_depth, freq, cfg->samplerate);
        }
        const f64 dt = freq / cfg->samplerate;

        switch(osc->waveform_id){
            default: break;
            case PULSE_POLY:{
                generated = poly_square(
                    v->amplitude, dt, osc->phase, osc->spec.coefficient
                ) * osc->spec.volume;
            }break;
            case TRIANGLE_POLY:{
                generated = poly_triangle(
                    v->amplitude, dt, osc->phase,&osc->integrator, &osc->dcx, 
                    &osc->dcy, dcblock
                ) * osc->spec.volume;
            }break;
            case SAW_POLY:{
                generated = poly_saw(
                    v->amplitude, dt, osc->phase
                ) * osc->spec.volume;
            }break;
            case SAW_RAW:{
                generated = quantize(sawtooth(
                    v->amplitude, osc->phase
                ), cfg->quantize_depth) * osc->spec.volume;
            }break;
            case TRIANGLE_RAW:{
                generated = quantize(triangle(
                    v->amplitude, osc->phase
                ), cfg->quantize_depth) * osc->spec.volume;
            }break;
            case PULSE_RAW:{
                generated = quantize(square(
                    v->amplitude, osc->phase, osc->spec.coefficient
                ), cfg->quantize_depth) * osc->spec.volume;
            }break;
        }

        if(generated == 0.0) continue;

        osc->phase += dt;
        if(osc->phase >= 1.0) {
            osc->phase -= 1.0;
        }
        osc->time += 1.0 / cfg->samplerate;
        sum += generated / v->l.oscilators;
    }
    return sum;
}

static f32 loop_voicings(struct voice_control *vc, f64 wave_samples[VOICE_MAX]){
    f32 sample = 0.0;
    for(u32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &vc->voices[i];
        wave_samples[i] = 0.0;

        const bool cond1 = (v->active && v->env.state != ENVELOPE_OFF);
        const bool cond2 = (!v->active && v->env.state == ENVELOPE_RELEASE);
        if(cond1 || cond2){
            wave_samples[i] = loop_oscilators(v, &vc->cfg, vc->dcblock);
            wave_samples[i] *= adsr(
                &v->env.state, 
                &v->env.envelope, 
                &v->env.release_increment, 
                &vc->cfg
            );
        }

        if(wave_samples[i] == 0.0) continue;
        sample += (f32)tanh(wave_samples[i] * (f64)vc->cfg.sample_gain / VOICE_MAX) * vc->cfg.volume;
    }
    return sample;
}

static void loop_samples(size_t count, f32 *samplebuffer, struct voice_control *vc){
    for(size_t n = 0; n < count; n++){
        f64 wave_samples[VOICE_MAX];
        samplebuffer[n] = loop_voicings(
            vc, wave_samples
        ) * vc->cfg.volume;
    }
}

void stream_callback(void *data, SDL_AudioStream *stream, i32 add, i32 total){
    struct voice_control *vc = (struct voice_control *)data;
    if(!vc) { return; }

    size_t sample_count = (u32)add / sizeof(f32);
    while(sample_count > 0){
        f32 samples[SAMPLE_PER_CALLBACK];
        memset(samples, 0, sizeof(f32) * SAMPLE_PER_CALLBACK);
        const size_t valid_samples = SDL_min(sample_count, SDL_arraysize(samples));
        loop_samples(valid_samples, samples, vc);
        if(vc->dl.active){
            loop_delay(valid_samples, samples, vc);
        }
        stream_feed(stream, samples, (i32)valid_samples * (i32)sizeof(f32));
        render_push(samples, valid_samples, vc->render_buffer, vc->rbuflen);
        sample_count -= valid_samples;
    }
}

SDL_AudioSpec make_audio_spec(i32 channels, i32 samplerate){
    SDL_AudioSpec spec = {
        SDL_AUDIO_F32,
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

bool audio_stream_clear(SDL_AudioStream *stream){
    if(stream){
        if(!SDL_ClearAudioStream(stream)){
            printf("Could not clear queued audio: %s\n", SDL_GetError());
            return false;
        }
        return true;
    }
    return false;
}

// only call from main so it's still easy to free resources upon failure
bool audio_steam_ch_input_spec(SDL_AudioStream *stream, u32 dev_id, const SDL_AudioSpec *internal){
    if(dev_id){
        if(!pause_audio(dev_id)){
            return false;
        }
        
        if(!audio_stream_clear(stream)){
            return false;
        }

        audio_stream_unbind(stream);
        if(!set_audio_stream_format(stream, internal, NULL)){
            close_audio_device(dev_id);
            audio_stream_destroy(stream);
            return false;
        }

        if(!audio_stream_bind(stream, dev_id)){
            return false;
        }

        if(!resume_audio(dev_id)){
            return false;
        }

        return true;
    }

    return false;
}

bool set_audio_stream_format(SDL_AudioStream *stream, const SDL_AudioSpec *internal, const SDL_AudioSpec *output){
    if(!stream){
        return false;
    }
    
    if(!SDL_SetAudioStreamFormat(stream, internal, output)){
        printf("Failed to update stream format: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

SDL_AudioStream *audio_stream_create(const SDL_AudioSpec *internal, const SDL_AudioSpec *output){
    SDL_AudioStream *stream = SDL_CreateAudioStream(internal, output);
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

bool resume_audio(u32 id){
    if(id){
        if(!SDL_ResumeAudioDevice(id)){
            printf("Failed to resume: %s\n", SDL_GetError());
            return false;
        }
        return true;
    }
    return false;
}

bool pause_audio(u32 id){
    if(id){
        if(!SDL_PauseAudioDevice(id)){
            printf("Failed to pause: %s\n", SDL_GetError());
            return false;
        }
        return true;
    }
    return false;
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