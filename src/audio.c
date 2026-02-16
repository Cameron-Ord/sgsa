#include "../include/audio.h"
#include "../include/waveform.h"
#include <SDL3/SDL_audio.h>
#include <stdio.h>
#include <math.h>

const f64 DUTY_CYCLE = 0.25;
const f64 VIBRATO_ON = 0.18;
const f32 VOLUME = 1.0f;
const i32 SAMPLE_PER_CALLBACK = 128;
const f32 MASTER_GAIN = 0.75f;
const f64 VRATE = 5.5;
const f64 EFFECT_DEPTH = 4.25;
const i32 BIT_DEPTH = 8;

//const f64 alpha = 1.0 / SAMPLE_RATE;

bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len){
    return SDL_PutAudioStreamData(stream, samples, len);
}

static f32 loop_voicings(struct voice voices[VOICE_MAX], f64 wave_samples[VOICE_MAX], i32 wfid, i32 samplerate){
    f32 sample = 0.0;
    for(u32 i = 0; i < VOICE_MAX; i++){
        struct voice *v = &voices[i];
        wave_samples[i] = 0.0;

        if(v->env.state != ENVELOPE_OFF){
            f64 freq = v->osc.freq;
            if(v->osc.time > VIBRATO_ON){
                freq = vibrato(VRATE, EFFECT_DEPTH, freq, samplerate);
            }
            const f64 dt = freq / samplerate;

            switch(wfid){
                default:break;
                case PULSE_RAW:{
                    wave_samples[i] = quantize(square(v->osc.amplitude, v->osc.phase, DUTY_CYCLE), BIT_DEPTH);
                }break;
                case SAW_RAW:{
                    wave_samples[i] = quantize(sawtooth(v->osc.amplitude, v->osc.phase), BIT_DEPTH);
                }break;
                case TRIANGLE_RAW:{
                    wave_samples[i] = quantize(triangle(v->osc.amplitude, v->osc.phase), BIT_DEPTH);
                }break;
            }

            adsr(&v->env.state, &v->env.envelope, &v->env.release_increment, samplerate);
            wave_samples[i] *= v->env.envelope;

            v->osc.phase += dt;
            if(v->osc.phase >= 1.0){
                v->osc.phase -= 1.0;
            }
            v->osc.time += 1.0 / samplerate;
        }
        sample += (f32)wave_samples[i];
    }
    return sample;
}

static void loop_samples(size_t count, f32 *samplebuffer, struct voice_control *vc){
    for(size_t n = 0; n < count; n++){
        f32 sample = 0.0;
        f64 wave_samples[VOICE_MAX];
        sample += loop_voicings(vc->voices, wave_samples, vc->waveform_id, vc->fmt.SAMPLE_RATE);
        samplebuffer[n] = tanhf(sample * MASTER_GAIN) * VOLUME;
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