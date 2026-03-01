#ifndef SGSA_HPP
#define SGSA_HPP
#include "typedef.hpp"
#include <SDL3/SDL.h>
#include <portmidi.h>
#include <string>
#include <vector>

#define PI 3.141592653589793f
#define VOICE_ON (1 << 0)
#define VOICE_OFF (1 << 1)
#define ENVELOPE_OFF (1 << 2)
#define ENVELOPE_ATTACKING (1 << 3)
#define ENVELOPE_DECAYING (1 << 4)
#define ENVELOPE_RELEASING (1 << 5)
#define ENVELOPE_SUSTAINING (1 << 6)
#define ENVELOPE_ON (1 << 7)

// (VALUE - VALUE) / SAMPLES
#define ATTACK_INCREMENT(samplerate, ATK) \
    1.0f / ((ATK) * (samplerate))
#define DECAY_INCREMENT(samplerate, DEC, SUS) \
    (1.0f - (SUS)) / ((DEC) * (samplerate))
#define RELEASE_INCREMENT(samplerate, REL) \
    (REL) <= 0.0f ? 1.0f : 1.0f / ((REL) * (samplerate))

#define NYQUIST(samplerate) (samplerate) / 2.0f

void stream_get(void *data, SDL_AudioStream *stream, i32 add, i32 total);

enum input_positions {
    INPUT_MSG_STATUS,
    INPUT_MSG_ONE,
    INPUT_MSG_TWO,
    INPUT_END
};

enum lfo_enum {
  MODE_VIBRATO,
  MODE_TREMOLO,
};

enum max_values {
  CONTROLLER_NAME_MAX = 256,
  CHANNEL_MAX = 2,
  MAX_TABLE_SIZE = 512,
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
    STATE_TIME,
//    STATE_INTEGRATOR,
//    STATE_DC_X,
//    STATE_DC_Y,
    STATE_END
};

enum table_enum {
    TABLE_SAW = 0,
    TABLE_END = 1,
    TABLE_FREQ_LOW = 0,
    TABLE_FREQ_MID = 1,
    TABLE_FREQ_HIGH = 2,
    TABLE_FREQ_END = 3,
    OCTAVES = 12,
};

enum env_types {
  ENV_ADSR,
  ENV_PIANO,
};

struct Lfo_Params {
  Lfo_Params(void);
  Lfo_Params(f32 RATE, f32 DEPTH, f32 TIMER, u8 MODE);
  f32 rate, depth, timer;
  u8 mode;
};

struct Env_Params {
  Env_Params(void);
  Env_Params(f32 ATK, f32 DEC, f32 SUS, f32 REL, std::string TYPE);
  f32 attack, decay, sustain, release;
  std::string type;
  size_t env_id;
};

struct Audio_Params {
  Audio_Params(void);
  Audio_Params(i32 CHANNELS, i32 SAMPLE_RATE, size_t VOICE_COUNT, size_t WT_SIZE, f32 TEMPO, f32 NOTE_DUR, f32 CUTOFF_LOW, f32 CUTOFF_HIGH);

  i32 channels, sample_rate;
  size_t voicings, wave_table_size;
  f32 tempo, note_duration;
  f32 lpf_alpha_low, lpf_alpha_high;
};

struct Params {
  Params(void);
  Params(Lfo_Params lfop_tmp, Env_Params envp_tmp, Audio_Params ap_tmp);
  struct Lfo_Params lfop;
  struct Env_Params envp;
  struct Audio_Params ap;
};

struct Interpolator {
  Interpolator(void);
  f32 unfiltered[CHANNEL_MAX];
  f32 high[CHANNEL_MAX];
  f32 low[CHANNEL_MAX];
  f32 filtered[CHANNEL_MAX];
  void lerp(f32 alpha_low, f32 alpha_high, i32 c);
};

struct Lfo {
  Lfo(f32 r, f32 d, f32 t, i32 sr, u8 m);
  u8 mode;
  f32 rate, depth, timer;
  f32 inc, max;
  f32 phase; 
  f32 vibrato(void);
  void increment_lfo(void);
};

struct Oscilator {
  Oscilator(i32 sr, const Env_Params& env, const Lfo_Params& lfop);
  const Env_Params& env_;
  const Lfo_Params& lfop_;

  f32 gen_states[STATE_END];
  struct Interpolator samples;
  u8 env_state;
  const Lfo lfo;

  void adsr(i32& counter, i32 samplerate, f32 atk, f32 dec, f32 sus, f32 rel);
  void increment_phase(f32 inc, f32 max);
  void increment_time(f32 inc);
};

struct Wave_Table {
    Wave_Table(i32 sample_rate, size_t table_size);
    f32 tables[TABLE_END][OCTAVES][MAX_TABLE_SIZE];
    f32 freq_mapper[OCTAVES];
    size_t size;
    void print_table(f32 table[MAX_TABLE_SIZE]);
    u8 index_octave(f32 freq) const;
};

struct Oscilator_Cfg{
  Oscilator_Cfg(void);
  Oscilator_Cfg(f32 cyc, std::string n, f32 det, f32 vol, f32 st);
  f32 cycle;
  std::string name;
  f32 detune, volume, step;
  size_t table_id;
};

struct Voice {
    Voice(i32 sr, size_t osc_c, const Env_Params& env, const Lfo_Params& lfop, std::vector<Oscilator_Cfg> templates);
    i32 sample_rate;
    const Env_Params& env_;
    const Lfo_Params& lfop_;

    u8 voice_state;
    i32 active_oscilators;
    f32 freq;
    i32 midi_key;

    size_t osc_count;
    std::vector<struct Oscilator_Cfg> cfgs;
    std::vector<struct Oscilator> oscs;
};

struct Audio_Data {
    Audio_Data(
      const Params& p,
      std::vector<Oscilator_Cfg> templates,
      size_t osc_c
    );
    const Lfo_Params& lfop_;
    const Audio_Params& ap_;
    const Env_Params& envp_;

    std::vector<struct Voice> voices;
    const struct Wave_Table wave_table;
};

// This is where it all happens basically
// Params are passed by value and on construction the Audio class constructs everything
// So literally make any changes anywhere (like the Params struct or oscilator cfg)
// and you can just reinstantiate this class and it will destroy then recreate
// All the audio allocations. Goes crazy
//
// Params is stored by copying, the Oscilator_Cfg vector is just passed by value since 
// it's just a template for values nested inside, then is thrown away since it gets copy assigned to a location
class Audio {
public:
    Audio(Params p, std::vector<Oscilator_Cfg> templates);
    ~Audio(void) = default;
    bool open(void);
    void close(void);
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
    struct Audio_Data &get_data(void) { return data; }
private:
    struct Params parameters;
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

    bool open(void);
    bool close(void);
    void list_available_controllers(void);
    void get_midi_device_by_name(void);
    bool open_stream(i32 bufsize);
    bool close_stream(void);
    void read_input(i32 len);
    void clear_msg_buf(void);
    void iterate_input_on(struct Audio_Data &data, i32 midi_key);
    void iterate_input_off(struct Audio_Data &data, i32 midi_key);

    const i32 *get_msgbuf(void) const { return msgbuf; }

private:
    std::string input_name;
    i32 input_id;
    PortMidiStream *stream;
    i32 msgbuf[INPUT_END];
};

class Manager {
public:
    Manager(const char *name_arg, const Params params, std::vector<Oscilator_Cfg> templates);
    ~Manager(void) = default;
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
