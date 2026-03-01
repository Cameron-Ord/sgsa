#ifndef AUDIO_HPP
#define AUDIO_HPP
#include "config.hpp"
#include "typedef.hpp"

#include <vector>

#include <SDL3/SDL.h>

#define PI 3.141592653589793f
// (VALUE - VALUE) / SAMPLES
#define ATTACK_INCREMENT(samplerate, ATK) \
    1.0f / ((ATK) * (samplerate))
#define DECAY_INCREMENT(samplerate, DEC, SUS) \
    (1.0f - (SUS)) / ((DEC) * (samplerate))
#define RELEASE_INCREMENT(samplerate, REL) \
    (REL) <= 0.0f ? 1.0f : 1.0f / ((REL) * (samplerate))

#define NYQUIST(samplerate) (samplerate) / 2.0f

void stream_get(void *data, SDL_AudioStream *stream, i32 add, i32 total);

enum ENV_STATE : size_t {
  ATK,
  DEC,
  REL,
  SUS,
  OFF
};

//    STATE_INTEGRATOR,
//    STATE_DC_X,
//    STATE_DC_Y,
enum OSC_STATE : size_t {
    ENVELOPE,
    PHASE, 
    TIME,
    STATE_COUNT
};

struct Interpolator {
  Interpolator(void);
  f32 unfiltered[MAX::CHANNEL_MAX];
  f32 high[MAX::CHANNEL_MAX];
  f32 low[MAX::CHANNEL_MAX];
  f32 filtered[MAX::CHANNEL_MAX];
  void lerp(f32 alpha_low, f32 alpha_high, i32 c);
};

struct Lfo {
  Lfo(f32 r, f32 d, f32 t, i32 sr, LFO_TYPE m);
  LFO_TYPE mode;
  f32 rate;
  f32 depth;
  f32 timer;
  f32 inc;
  f32 max;
  f32 phase; 
  f32 vibrato(void);
  void increment_lfo(void);
};

struct Oscilator {
  Oscilator(i32 sr, const Env_Params& env, const Lfo_Params& lfop);
  const Env_Params& env_;
  const Lfo_Params& lfop_;

  f32 gen_states[OSC_STATE::STATE_COUNT];
  struct Interpolator samples;
  u8 env_state;
  const Lfo lfo;

  void ar(i32& counter, i32 samplerate, f32 atk, f32 rel);
  void adsr(i32& counter, i32 samplerate, f32 atk, f32 dec, f32 sus, f32 rel);
  void increment_phase(f32 inc, f32 max);
  void increment_time(f32 inc);
};

struct Wave_Table {
    Wave_Table(i32 sample_rate, size_t table_size);
    f32 tables[WAVEFORM_TYPE::WAVEFORM_COUNT][MAX::OCTAVES][MAX::MAX_TABLE_SIZE];
    f32 freq_mapper[MAX::OCTAVES];
    size_t size;
    void print_table(f32 table[MAX::MAX_TABLE_SIZE]);
    size_t index_octave(f32 freq) const;
};

struct Voice {
    Voice(i32 sr, size_t osc_c, const Env_Params& env, const Lfo_Params& lfop, std::vector<Oscilator_Cfg> templates);
    i32 sample_rate;
    const Env_Params& env_;
    const Lfo_Params& lfop_;
 
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

#endif
