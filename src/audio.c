#include "../include/audio.h"
#include "../include/waveform.h"
#include <stdio.h>

const i32 SAMPLE_PER_CALLBACK = 128;

bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len){
    return SDL_PutAudioStreamData(stream, samples, len);
}

void stream_callback(void *userdata, SDL_AudioStream *stream, i32 add, i32 total){
    struct voice *voices = (struct voice *)userdata;
    if(!voices) { return; }

    u32 sample_count = (u32)add / sizeof(f32);
    while(sample_count > 0){
        f32 samples[SAMPLE_PER_CALLBACK];
        memset(samples, 0, sizeof(samples));
        
        const u32 valid_samples = SDL_min(sample_count, SDL_arraysize(samples));
        for(u32 i = 0; i < valid_samples; i++){
            f64 sample = 0.0;
            i32 active_count = 0;

            for(u32 j = 0; j < VOICE_MAX; j++){
                struct voice *voice = &voices[j];
                if(voice->active){
                    voice->time += 1.0 / SAMPLE_RATE;
                    sample += sawtooth(voice->time, voice->freq);
                    active_count++;
                }
            }
            if(active_count > 0) sample /= active_count;

            samples[i] = (f32)sample * 1.0f;
        }

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

bool set_audio_callback(SDL_AudioStream *stream, struct voice *voices){
    if(!SDL_SetAudioStreamGetCallback(stream, stream_callback, voices)){
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