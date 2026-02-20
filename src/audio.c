#include "../include/audio.h"
#include "../include/waveform.h"
#include "../include/effect.h"
#include "../include/util.h"

#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include <stdio.h>
#include <math.h>

#include <assert.h>
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
        const f32 mixed = samples[i] + tanhf(0.5f * delayed * vc->cfg.delay_gain);
        samples[i] = mixed;
        delay_line_write(samples[i], &vc->dl);
    }
}

static void loop_oscilators(struct voice *v, f64 sum[],const struct configs *cfg, f64 dcblock){
    for(i32 c = 0; c < cfg->channels; c++){
        sum[c] = 0.0;
    }

    for(u32 i = 0; i < v->l.oscilators; i++){
        struct envelope *env = &v->l.osc[i].env;
        struct oscilator *osc = &v->l.osc[i];

        const bool first = v->active && env->state != ENVELOPE_OFF;
        const bool second = !v->active && env->state == ENVELOPE_RELEASE;
        const bool cond = first || second;
        if(!cond) {
            continue;
        }

        f64 freq = 0.0;
        switch((u8)osc->spec.detuned){
            default: break;
            case 0: {
                freq = v->l.base_freq * osc->spec.octave_increment;
            }break;

            case 1:{
                freq = v->l.base_freq * osc->spec.octave_increment * osc->spec.detune;
            }break;
        }

        if(osc->time > cfg->vibrato_on){
            freq = vibrato(cfg->vibration_rate, cfg->vibration_depth, freq, cfg->samplerate);
        }
        const f64 inc = freq / cfg->samplerate;
        const f64 dt = 1.0 / cfg->samplerate;

        switch(osc->waveform_id){
            default: break;
            case SINE:{
                for(i32 c = 0; c < cfg->channels; c++){
                    osc->generated[c] = sine(v->amplitude, osc->phase) * osc->spec.volume;
                }
            }break;
            case PULSE_POLY:{
                for(i32 c = 0; c < cfg->channels; c++){
                    osc->generated[c] = poly_square(v->amplitude, inc, osc->phase, osc->spec.coefficient) * osc->spec.volume;
                }
            }break;
            case TRIANGLE_POLY:{
                for(i32 c = 0; c < cfg->channels; c++){
                    osc->generated[c] = poly_triangle(v->amplitude, inc, osc->phase,&osc->integrator, &osc->dcx, &osc->dcy, dcblock) * osc->spec.volume;
                }
            }break;
            case SAW_POLY:{
                for(i32 c = 0; c < cfg->channels; c++){
                    osc->generated[c] = poly_saw(v->amplitude, inc, osc->phase) * osc->spec.volume;
                }
            }break;
            case SAW_RAW:{
                for(i32 c = 0; c < cfg->channels; c++){
                    osc->generated[c] = sawtooth(v->amplitude, osc->phase) * osc->spec.volume;
                }
            }break;
            case TRIANGLE_RAW:{
                for(i32 c = 0; c < cfg->channels; c++){
                    osc->generated[c] = triangle(v->amplitude, osc->phase)* osc->spec.volume;
                }
            }break;
            case PULSE_RAW:{
                for(i32 c = 0; c < cfg->channels; c++){
                    osc->generated[c] = square(v->amplitude, osc->phase, osc->spec.coefficient) * osc->spec.volume;
                }
            }break;
        }
        // inc (ie: phase increment in seconds) / (cutoff_in_seconds + inc)
        const f64 alpha_high = (dt) / (CUTOFF_TO_SEC(HZ_TO_RAD(15000.0)) + dt);
        const f64 alpha_low = (dt) / (CUTOFF_TO_SEC(HZ_TO_RAD(80.0)) + dt);
        // Basic interpolation using a cutoff alpha
        adsr(env, cfg->samplerate);
        for(i32 c = 0; c < cfg->channels; c++){
            //adsr and compress per osc
            osc->generated[c] *= env->envelope;
            osc->generated[c] = tanh(osc->generated[c] * (f64)cfg->osc_gain);
            //filter
            osc->filtered_high[c] += linear_interpolate(osc->generated[c], osc->filtered_high[c], alpha_high);
            osc->filtered_low[c] += linear_interpolate(osc->generated[c], osc->filtered_low[c], alpha_low);
            //mix and sum
            osc->generated[c] = osc->filtered_high[c] - osc->filtered_low[c];
            sum[c] += osc->generated[c] / v->l.oscilators;
        }
        //const f64 out = 0.7 * osc->filtered_low + 0.3 * osc->filtered_high;
        osc->phase += inc;
        if(osc->phase >= 1.0) {
            osc->phase -= 1.0;
        }
        osc->time += 1.0 / cfg->samplerate;
    }
    //compress sum
    for(i32 c = 0; c < cfg->channels; c++){
        sum[c] = tanh(sum[c] * (f64)cfg->sample_gain);
    }
}

static void loop_voicings(struct voice_control *vc, f64 wave_samples[CHANNEL_MAX]){
    for(i32 c = 0; c < CHANNEL_MAX; c++){
        wave_samples[c] = 0.0;
    }
    for(u32 v = 0; v < VOICE_MAX; v++){
        switch(vc->cfg.channels){
            default: break;
            case MONO:{
                f64 sum[vc->cfg.channels];
                loop_oscilators(&vc->voices[v], sum,&vc->cfg, vc->dcblock);
                wave_samples[0] += (sum[0] / VOICE_MAX) * (f64)vc->cfg.volume;
            }break;

            case STEREO:{
                f64 angle = (0.5 * (PI / 2.0));
                f64 left = cos(angle);
                f64 right = sin(angle);

                f64 sum[vc->cfg.channels];
                loop_oscilators(&vc->voices[v], sum,&vc->cfg, vc->dcblock);
                wave_samples[0] += ((sum[0] * left) / VOICE_MAX) * (f64)vc->cfg.volume;
                wave_samples[1] += ((sum[1] * right) / VOICE_MAX) * (f64)vc->cfg.volume;
            }break;
        }
    }
}

static void loop_samples(size_t count, f32 *samplebuffer, struct voice_control *vc){
    for(size_t n = 0; n < count / (size_t)vc->cfg.channels; n++){
        f64 wave_samples[CHANNEL_MAX];
        loop_voicings(vc, wave_samples);
        for(i32 c = 0; c < vc->cfg.channels; c++){
            samplebuffer[n * (size_t)vc->cfg.channels + (size_t)c] = (f32)wave_samples[c] * vc->cfg.volume;
        }
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
    if(internal){
        printf("Created stream with internal spec: (%d %d %d)\n", internal->channels, internal->freq, internal->format);
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