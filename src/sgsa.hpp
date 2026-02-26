#ifndef SGSA_HPP
#define SGSA_HPP
#include "typedef.hpp"
#include <SDL3/SDL.h>
#include <portmidi.h>

void stream_get(void *data, SDL_AudioStream *stream, i32 add, i32 total);

class Audio {
public:
    Audio(i32 channels, i32 samplerate);
    ~Audio(void) = default;
    bool set_audio_callback(void *userdata);
    bool bind_stream(void);
    bool unbind_stream(void);
    bool open_audio_device(void);
    bool create_audio_stream(void);
    bool close_audio_device(void);
    bool destroy_audio_stream(void);
    bool resume(void);
    bool pause(void);
    void clear(void);
    void quit(void);
private:
    bool valid;
    u32 dev;
    SDL_AudioStream *stream;
    SDL_AudioSpec internal;
    SDL_AudioSpec output;
};

class KeyEvents {
public:
    KeyEvents(void) = default;
    ~KeyEvents(void) = default;
private:
};

class Controller {
public:
    Controller(void);
    ~Controller(void) = default;

    void list_available_controllers(void);
private:
};

class Manager {
public:
    Manager(i32 channels, i32 samplerate);
    ~Manager(void);
    bool quit(void);

    Audio &get_audio(void) { return audio; };
    KeyEvents &get_events(void) { return key_events; }
    Controller &get_controller(void) { return controller; };
private:
    Audio audio;
    KeyEvents key_events;
    Controller controller;
};

#endif