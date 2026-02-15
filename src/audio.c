#include "../include/audio.h"
#include "../include/waveform.h"
#include <stdio.h>
#include <math.h>

const f32 volume = 0.8f;
const i32 SAMPLE_PER_CALLBACK = 128;
const f32 MASTER_GAIN = 0.75f;
const f64 alpha = 1.0 / SAMPLE_RATE;
f64 interpolated_gain = 0.0;

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
    struct voice *voices = (struct voice *)data;
    if(!voices) { return; }

    u32 sample_count = (u32)add / sizeof(f32);
    while(sample_count > 0){
        f32 samples[SAMPLE_PER_CALLBACK];
        memset(samples, 0, sizeof(samples));
        
        const u32 valid_samples = SDL_min(sample_count, SDL_arraysize(samples));
        for(u32 i = 0; i < valid_samples; i++){
            f64 sample = 0.0;
            f64 wave_samples[VOICE_MAX];

            for(u32 j = 0; j < VOICE_MAX; j++){
                struct voice *v = &voices[j];
                wave_samples[j] = 0.0;

                if(v->state != ENVELOPE_OFF){
                    v->phase += v->freq / SAMPLE_RATE;
                    while(v->phase >= 1.0) v->phase -= 1.0;
                    
                    switch(v->waveform_id){
                        default:break;
                        case SINE:{
                            wave_samples[j] = sine(v->phase);
                        }break;
                        case FOURIER_ST:{
                            wave_samples[j] = fourier_sawtooth(v->phase, v->freq);
                        }break;
                        case R_FOURIER_ST:{
                            wave_samples[j] = reverse_fourier_sawtooth(v->phase, v->freq);
                        }break;
                        case FOURIER_PULSE:{
                            wave_samples[j] = fourier_pulse(v->phase, v->freq, 0.25);
                        }break;
                        case SQUARE:{
                            wave_samples[j] = square(v->phase, 0.25);
                        }break;
                        case FOURIER_SQUARE:{
                            wave_samples[j] = fourier_square(v->phase, v->freq);
                        }break;
                        case TRIANGLE:{
                            wave_samples[j] = triangle(v->phase);
                        }break;
                    }

                    const f64 env = adsr(&v->state, &v->envelope, &v->release_increment);
                    wave_samples[j] = wave_samples[j] * env;
                    sample += wave_samples[j];
                }
            }
            //const f32 compressed_arctan = (2.0f / (f32)PI) * atanf((f32)sample * (f32)MASTER_GAIN);
            const f32 compressed_tanh = tanhf((f32)sample * MASTER_GAIN);
            samples[i] = compressed_tanh * volume;
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

bool set_audio_callback(SDL_AudioStream *stream, struct voice *data){
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