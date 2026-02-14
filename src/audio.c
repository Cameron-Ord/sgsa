#include "../include/audio.h"
#include "../include/waveform.h"
#include "../include/render.h"
#include <stdio.h>
#include <math.h>

const f64 volume = 1.0;
const i32 SAMPLE_PER_CALLBACK = 128;
const f64 MASTER_GAIN = 0.9;
const f64 alpha = 0.0025;
f64 interpolated_gain = 0.0;

static void render_frame_push(const size_t samples, const f32 *src, f32 *dst){
    if(samples > 0 && src && dst){
        memmove(dst, dst + samples, (FRAME_RESOLUTION - samples) * sizeof(f32));
        memcpy(dst + (FRAME_RESOLUTION - samples), src, samples * sizeof(f32));
    }
}

// linear adsr
static f64 adsr(i32 *state, f64 *envelope, const f64 *release){
    f64 mutated = *envelope;
    switch(*state){
        default:break;
        case ENVELOPE_ATTACK:{
            mutated += ATTACK_INCREMENT;
            if(mutated >= 1.0){
                mutated = 1.0;
                *state = ENVELOPE_DECAY;
            }
        }break;
    
        case ENVELOPE_SUSTAIN: break;

        case ENVELOPE_DECAY: {
            mutated -= DECAY_INCREMENT;
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

bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len){
    return SDL_PutAudioStreamData(stream, samples, len);
}

void stream_callback(void *data, SDL_AudioStream *stream, i32 add, i32 total){
    struct userdata *ud = (struct userdata *)data;
    if(!ud) { return; }

    u32 sample_count = (u32)add / sizeof(f32);
    while(sample_count > 0){
        f32 samples[SAMPLE_PER_CALLBACK];
        memset(samples, 0, sizeof(samples));
        
        const u32 valid_samples = SDL_min(sample_count, SDL_arraysize(samples));
        for(u32 i = 0; i < valid_samples; i++){
            f64 sample = 0.0;
            f64 wave_samples[VOICE_MAX];
            u32 active_count = 0;

            for(u32 j = 0; j < VOICE_MAX; j++){
                struct voice *v = &ud->voices[j];
                wave_samples[j] = 0.0;

                if(v->state != ENVELOPE_OFF){
                    v->phase += v->freq / SAMPLE_RATE;
                    if(v->phase >= 1.0) v->phase -= 1.0;

                    wave_samples[j] = v->waveform(v->phase, v->freq) * adsr(&v->state, &v->envelope, &v->release_increment);
                    active_count++;
                }
            }

            for(u32 k = 0; k < VOICE_MAX; k++){
                if(wave_samples[k] == 0.0) continue;
                const f64 gain = 1.0 / active_count;
                interpolated_gain += linear_interpolate(gain, interpolated_gain, alpha);
                wave_samples[k] *= interpolated_gain;
                sample += wave_samples[k];
            }

            samples[i] = (f32)tanh(sample * MASTER_GAIN);
        }

        stream_feed(stream, samples, (i32)valid_samples * (i32)sizeof(f32));
        render_frame_push(valid_samples, samples, ud->frame->samples);
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

struct playback_device open_audio_device(SDL_AudioSpec output_spec){
    u32 id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &output_spec);
    if(!id){
        return (struct playback_device){ 0, NULL, { 0, 0, 0 }, false };
    }
    return (struct playback_device){ id, NULL, output_spec, true };
}

SDL_AudioStream *audio_stream_create(SDL_AudioSpec input, SDL_AudioSpec output){
    SDL_AudioStream *stream = SDL_CreateAudioStream(&input, &output);
    if(!stream){
        printf("%s\n", SDL_GetError());
        return NULL;
    }
    return stream;
}

bool set_audio_callback(SDL_AudioStream *stream, struct userdata *data){
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