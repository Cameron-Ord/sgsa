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
        const f32 delayed = delay_line_read(&vc->dl) * vc->cfg.entries[DELAY_FEEDBACK].value;
        const f32 mixed = samples[i] + tanhf(0.5f * delayed * vc->cfg.entries[DELAY_GAIN].value);
        samples[i] = mixed;
        delay_line_write(samples[i], &vc->dl);
    }
}

static void loop_oscilators(struct voice *v, f32 sum[],const struct configs *cfg, f32 dcblock){
    const i32 channels = (i32)cfg->entries[CHANNELS].value;
    const i32 samplerate = (i32)cfg->entries[SAMPLE_RATE].value;
    
    for(i32 c = 0; c < channels; c++){
        sum[c] = 0.0f;
    }

    for(u32 i = 0; i < v->l.oscilators; i++){
        struct oscilator *osc = &v->l.osc[i];
        
        struct osc_entry *env = osc->env.entries;
        struct osc_entry *spec = osc->spec.entries;
        f32 *state = osc->state.entries;

        const bool first = v->active && env[STATE].value != (f32)ENVELOPE_OFF;
        const bool second = !v->active && env[STATE].value == (f32)ENVELOPE_RELEASE;
        const bool cond = first || second;
        if(!cond) {
            continue;
        }
        f32 freq = v->l.base_freq * spec[OCTAVE].value * spec[DETUNE].value;

        if(state[TIME] > cfg->entries[VIBRATO_ONSET].value){
            freq = vibrato(
                cfg->entries[VIBRATO_RATE].value, 
                cfg->entries[VIBRATO_DEPTH].value, freq, 
                cfg->entries[SAMPLE_RATE].value
            );
        }
        const f32 inc = freq / cfg->entries[SAMPLE_RATE].value;
        const f32 dt = 1.0f / cfg->entries[SAMPLE_RATE].value;

        switch(osc->waveform_id){
            default: break;
            case SINE:{
                for(i32 c = 0; c < channels; c++){
                    osc->gen.generated[c] = sine(
                        v->amplitude, 
                        state[PHASE]
                    ) * spec[OSC_VOLUME].value;
                }
            }break;
            case PULSE_POLY:{
                for(i32 c = 0; c < channels; c++){
                    osc->gen.generated[c] = poly_square(
                        v->amplitude, 
                        inc, 
                        state[PHASE], 
                        spec[COEFF].value
                    ) * spec[OSC_VOLUME].value;
                }
            }break;
            case TRIANGLE_POLY:{
                for(i32 c = 0; c < channels; c++){
                    osc->gen.generated[c] = poly_triangle(v->amplitude, inc, state[PHASE],&state[INTEGRATOR], &state[DC_X], &state[DC_Y], dcblock) * spec[OSC_VOLUME].value;
                }
            }break;
            case SAW_POLY:{
                for(i32 c = 0; c < channels; c++){
                    osc->gen.generated[c] = poly_saw(v->amplitude, inc, state[PHASE]) * spec[OSC_VOLUME].value;
                }
            }break;
            case SAW_RAW:{
                for(i32 c = 0; c < channels; c++){
                    osc->gen.generated[c] = sawtooth(v->amplitude, state[PHASE]) * spec[OSC_VOLUME].value;
                }
            }break;
            case TRIANGLE_RAW:{
                for(i32 c = 0; c < channels; c++){
                    osc->gen.generated[c] = triangle(v->amplitude, state[PHASE])* spec[OSC_VOLUME].value;
                }
            }break;
            case PULSE_RAW:{
                for(i32 c = 0; c < channels; c++){
                    osc->gen.generated[c] = square(v->amplitude, state[PHASE], spec[COEFF].value) * spec[OSC_VOLUME].value;
                }
            }break;
        }
        // inc (ie: phase increment in seconds) / (cutoff_in_seconds + inc)
        const f32 alpha_high = (dt) / (CUTOFF_IN_SEC(HZ_TO_RAD_PER_SEC(14000.0f)) + dt);
        const f32 alpha_low = (dt) / (CUTOFF_IN_SEC(HZ_TO_RAD_PER_SEC(80.0f)) + dt);
        // Basic interpolation using a cutoff alpha
        
        f32 *envelope_state = &env[STATE].value;
        f32 *envelope = &env[ENVELOPE].value;
        const f32 *atk = &env[ATTACK].value;
        const f32 *dec = &env[DECAY].value;
        const f32 *sus = &env[SUSTAIN].value;
        const f32 *rel = &env[RELEASE].value;

        adsr(envelope, envelope_state, atk, dec, sus, rel, samplerate);
        for(i32 c = 0; c < channels; c++){
            //adsr and compress per osc
            osc->gen.generated[c] *= *envelope;
            //filter
            osc->gen.filtered_high[c] += linear_interpolate(osc->gen.generated[c], osc->gen.filtered_high[c], alpha_high);
            osc->gen.filtered_low[c] += linear_interpolate(osc->gen.generated[c], osc->gen.filtered_low[c], alpha_low);
            //mix and sum
            osc->gen.generated[c] = osc->gen.filtered_high[c] - osc->gen.filtered_low[c];
            osc->gen.generated[c] = tanhf(osc->gen.generated[c] * (f32)cfg->entries[OSC_GAIN].value);
            sum[c] += osc->gen.generated[c] / (f32)v->l.oscilators;
        }
        //const f32 out = 0.7 * osc->filtered_low + 0.3 * osc->filtered_high;
        state[PHASE] += inc;
        if(state[PHASE] >= 1.0f) {
            state[PHASE] -= 1.0f;
        }
        state[TIME] += 1.0f / (f32)samplerate;
    }
    //compress sum
    for(i32 c = 0; c < channels; c++){
        sum[c] = tanhf(sum[c] * (f32)cfg->entries[SAMPLE_GAIN].value);
    }
}

static void loop_voicings(struct voice_control *vc, f32 wave_samples[CHANNEL_MAX]){
    for(i32 c = 0; c < CHANNEL_MAX; c++){
        wave_samples[c] = 0.0;
    }
    const i32 channels = (i32)vc->cfg.entries[CHANNELS].value;
    for(u32 v = 0; v < VOICE_MAX; v++){
        switch(channels){
            default: break;
            case MONO:{
                f32 sum[channels];
                loop_oscilators(&vc->voices[v], sum,&vc->cfg, vc->dcblock);
                wave_samples[0] += (sum[0] / VOICE_MAX);
            }break;

            case STEREO:{
                f32 angle = (0.5f * (PI / 2.0f));
                f32 left = cosf(angle);
                f32 right = sinf(angle);

                f32 sum[channels];
                loop_oscilators(&vc->voices[v], sum,&vc->cfg, vc->dcblock);
                wave_samples[0] += ((sum[0] * left) / VOICE_MAX);
                wave_samples[1] += ((sum[1] * right) / VOICE_MAX);
            }break;
        }
    }
}

static void loop_samples(size_t count, f32 *samplebuffer, struct voice_control *vc){
    const size_t channels = (size_t)vc->cfg.entries[CHANNELS].value;
    for(size_t n = 0; n < count / channels; n++){
        f32 wave_samples[CHANNEL_MAX];
        loop_voicings(vc, wave_samples);
        for(size_t c = 0; c < channels; c++){
            samplebuffer[n * channels + c] = wave_samples[c] * vc->cfg.entries[MAIN_VOLUME].value;
        }
    }
}

void stream_callback(void *data, SDL_AudioStream *stream, i32 add, i32 total){
    (void)total;
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