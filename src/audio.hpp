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

class LPF {
public:
  LPF(void);
  void lerp(f32 target[SIZES::CHANNEL_MAX], f32 alow, f32 ahigh, i32 c);
  const f32 *get_low(void) const { return low; }
  const f32 *get_high(void) const { return high; }
private:
  f32 high[SIZES::CHANNEL_MAX];
  f32 low[SIZES::CHANNEL_MAX];
};

class Lfo {
public:
  Lfo(void) = default;

  f32 vibrato(f32 depth);
  void increment_lfo(f32 inc);

private:
  f32 phase; 
};

class Oscilator {
public:
  Oscilator(void);

  f32 get_phase_val(void) const { return phase; }
  f32 get_time_val(void) const { return time; }
  void start(void);

  const Oscilator_Cfg get_cfg(void) const { return cfg; }

  void increment_phase(f32 inc, f32 max);
  void increment_time(f32 dt);
  f32* get_sample_array(void) { return gen; }

private:
  f32 gen[SIZES::CHANNEL_MAX];
  Oscilator_Cfg cfg;

  f32 phase;
  f32 time;
};

class Voice {
public:
    Voice(void);
    bool done(void) const;
    bool releasing(void) const;
    const std::vector<Oscilator>& get_osc_array(void) const { return oscs; }
    std::vector<Oscilator>& get_osc_array(void) { return oscs; }
    size_t get_osc_count(void) const { return oscs.size(); }
    i32 get_key(void) const { return midi_key; }
    void set_key(i32 key) { midi_key = key; }
    f32 get_freq(void) const { return freq; }
    void set_freq(f32 val) { freq = val; }
    i32 get_active_count(void) const { return active_oscilators; }
    void set_active_count(i32 val) { active_oscilators = val; }
    LPF& get_lpf(void) { return lpf; }

    u8 get_env_state(void) const { return env_state; }
    f32 get_envelope(void) const { return envelope; }
    void set_envelope(f32 val) { envelope = val; }
    void set_env_state(u8 state) { env_state = state; }
    void ar(i32 samplerate, f32 atk, f32 rel);
    void adsr(i32 samplerate, f32 atk, f32 dec, f32 sus, f32 rel);
private:
    i32 active_oscilators;
    f32 freq;
    i32 midi_key;
    u8 env_state;
    f32 envelope;
    std::vector<Oscilator> oscs;
    Lfo lfo;
    LPF lpf;
};

class Wave_Table {
public:
    Wave_Table(i32 sample_rate, size_t table_size);
    size_t index_octave(f32 freq) const;
    size_t get_size(void) const { return size; }
    const f32 *get_table(size_t id, size_t index) const;
private:
    f32 tables[WAVEFORM_TYPE::WAVEFORM_COUNT][SIZES::OCTAVES][SIZES::MAX_TABLE_SIZE];
    f32 freq_mapper[SIZES::OCTAVES];
    size_t size;

    void fourier_saw(void);
    void sine(void);
    void fourier_square(void);
};

class Synth {
public:
  Synth(void);
  const Audio_Params& get_audio_cfg(void) const { return audio_cfg; }
  const Env_Params& get_env_cfg(void) const { return env_cfg; }
  const Wave_Table& get_wave_table(void) const { return wave_table; }
  const std::vector<Voice>& get_voices(void) const { return voices; }
  std::vector<Voice>& get_voices(void) { return voices; }
  void loop_voicings_off(i32 midi_key);
  void loop_voicings_on(i32 midi_key);

private:
  Lfo_Params lfo_cfg;
  Env_Params env_cfg;
  Audio_Params audio_cfg;
  Wave_Table wave_table;
  std::vector<Voice> voices;
};

class Audio_Sys {
public:
    Audio_Sys(i32 chan, i32 sample_rate);
    ~Audio_Sys(void) = default;
    bool open(void *userdata);
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
private:
    u32 dev;
    SDL_AudioStream *stream;
    SDL_AudioSpec internal;
    SDL_AudioSpec output;
};

#endif
