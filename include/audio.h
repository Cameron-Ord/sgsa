#ifndef AUDIO_H
#define AUDIO_H
#include <SDL3/SDL_audio.h>
#include <stdbool.h>
#include "typedef.h"

struct voice_control;

struct playback_device {
    u32 id;
    SDL_AudioStream *stream;
    SDL_AudioSpec output_spec;
    bool valid;
};

bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 bytes);
void stream_callback(void *userdata, SDL_AudioStream *stream, i32 add, i32 total);

SDL_AudioSpec make_audio_spec(i32 channels, i32 samplerate);
struct playback_device open_audio_device(void);
bool pause_audio(u32 id);
bool resume_audio(u32 id);
void close_audio_device(u32 id);
bool audio_stream_clear(SDL_AudioStream *stream);
bool audio_steam_ch_input_spec(SDL_AudioStream *stream, u32 dev_id, const SDL_AudioSpec *internal);
bool set_audio_stream_format(SDL_AudioStream *stream, const SDL_AudioSpec *internal, const SDL_AudioSpec *device_out);
bool set_audio_callback(SDL_AudioStream *stream, struct voice_control *data);
SDL_AudioStream *audio_stream_create(const SDL_AudioSpec *internal, const SDL_AudioSpec *device_out);
void audio_stream_destroy(SDL_AudioStream *stream);
bool audio_stream_bind(SDL_AudioStream *stream, u32 id);
void audio_stream_unbind(SDL_AudioStream *stream);
#endif