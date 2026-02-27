#ifndef SGSA_HPP
#define SGSA_HPP
#include "typedef.hpp"
#include <SDL3/SDL.h>
#include <portmidi.h>
#include <memory>

#define VOICE_ON (1 << 1)
#define VOICE_OFF (1 << 2)
#define ENVELOPE_OFF (1 << 3)
#define ENVELOPE_ATTACKING (1 << 4)
#define ENVELOPE_DECAYING (1 << 5)
#define ENVELOPE_RELEASING (1 << 6)
#define ENVELOPE_SUSTAINING (1 << 7)

// (VALUE - VALUE) / SAMPLES
#define ATTACK_INCREMENT(samplerate, ATK) \
    (1.0f - 0.0f) / ((ATK) * (samplerate))
#define DECAY_INCREMENT(samplerate, DEC, SUS) \
    (1.0f - (SUS)) / ((DEC) * (samplerate))
#define RELEASE_INCREMENT(envelope, samplerate, REL) \
    (envelope) / ((REL) * (samplerate))


#define CHANNEL_MAX 2
#define CONTROLLER_NAME_MAX 256
#define TABLE_SIZE 32
#define MAX_VOICE 4

void stream_get(void *data, SDL_AudioStream *stream, i32 add, i32 total);

enum input_positions {
    INPUT_MSG_STATUS,
    INPUT_MSG_ONE,
    INPUT_MSG_TWO,
    INPUT_END
};

enum midi_input_mappings {
  NOTE_ON = 0x90,
  NOTE_OFF = 0x80,
  CONTROL = 0xB0,
  CONTROL_ON = 0x7F,
  CONTROL_OFF = 0x0,
};

enum state_positions {
    STATE_ENVELOPE,
    STATE_PHASE, 
    STATE_PHASE_MOD,
    STATE_TIME,
//    STATE_INTEGRATOR,
//    STATE_DC_X,
//    STATE_DC_Y,
    STATE_END
};

enum table_locations {
    TABLE_PULSE,
    TABLE_TRIANGLE,
    TABLE_SQUARE,
    TABLE_END
};

struct Wave_Table {
    Wave_Table(f32 duty_cycle_coeff);
    const f32 cycle;
    f32 tables[TABLE_END][TABLE_SIZE];
    u8 current_table;
    void print_table(f32 table[TABLE_SIZE]);
};

struct Voice {
    Voice(void);
    u8 voice_state;
    f32 freq;
    i32 midi_key;
    f32 generative_states[STATE_END];
    f32 gen[CHANNEL_MAX];
    void adsr(i32 samplerate, f32 atk, f32 dec, f32 sus, f32 rel);
    f32 vibrato(f32 depth);
};

struct Audio_Data {
    Audio_Data(i32 chan, i32 sr, f32 atk, f32 dec, f32 sus, f32 rel, f32 cyc, f32 vrate, f32 vdepth);
    const i32 channels, samplerate;
    const f32 attack, decay, sustain, release;
    const f32 vibrato_rate, vibrato_depth;
    struct Voice voices[MAX_VOICE];
    const struct Wave_Table wave_table;
};

class Audio {
public:
    Audio(i32 chan, i32 sr, f32 atk, f32 dec, f32 sus, f32 rel, f32 cyc, f32 vrate, f32 vdepth);
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
    struct Audio_Data &get_data(void) { return data; }
private:
    bool valid;
    u32 dev;
    SDL_AudioStream *stream;
    SDL_AudioSpec internal;
    SDL_AudioSpec output;
    struct Audio_Data data;
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
    void iterate_input_on(struct Audio_Data &data, i32 midi_key);
    void iterate_input_off(struct Audio_Data &data, i32 midi_key);

    const i32 *get_msgbuf(void) const { return msgbuf; }

private:
    char input_name[CONTROLLER_NAME_MAX + 1];
    i32 input_id;
    PortMidiStream *stream;
    i32 msgbuf[INPUT_END];
};

class Manager {
public:
    Manager(i32 channels, i32 samplerate, f32 atk, f32 dec, f32 sus, f32 rel, f32 cyc, f32 vrate, f32 vdepth, const char *name_arg);
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