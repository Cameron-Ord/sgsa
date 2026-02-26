#ifndef SGSA_HPP
#define SGSA_HPP
#include "typedef.hpp"
#include <SDL3/SDL.h>
#include <portmidi.h>

enum STATUS {
  NOTE_ON = 0x90,
  NOTE_OFF = 0x80,
  CONTROL = 0xB0,
  CONTROL_ON = 0x7F,
  CONTROL_OFF = 0x0,
};

#define CONTROLLER_NAME_MAX 256
void stream_get(void *data, SDL_AudioStream *stream, i32 add, i32 total);

enum input_positions {
    INPUT_MSG_STATUS,
    INPUT_MSG_ONE,
    INPUT_MSG_TWO,
    INPUT_END
};

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
    Controller(const char *name_arg);
    ~Controller(void) = default;

    void list_available_controllers(void);
    void get_midi_device_by_name(const char *name);
    bool open_stream(i32 bufsize);
    void read_input(i32 len);
    void clear_msg_buf(void);

    const i32 *get_msgbuf(void) const { return msgbuf; }
private:
    char input_name[CONTROLLER_NAME_MAX + 1];
    i32 input_id;
    PortMidiStream *stream;
    i32 msgbuf[INPUT_END];
};

class Manager {
public:
    Manager(i32 channels, i32 samplerate, const char *name_arg);
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