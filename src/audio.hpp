#ifndef AUDIO_HPP
#define AUDIO_HPP
#include "config.hpp"
#include "typedef.hpp"

#include <vector>

#include <SDL3/SDL.h>

#define PI 3.141592653589793f
// (VALUE - VALUE) / SAMPLES
#define ATTACK_INCREMENT(samplerate, ATK)                                      \
  1.0f / (((ATK) <= 0.0f ? 1.0f : (ATK)) * (samplerate))
#define DECAY_INCREMENT(samplerate, DEC, SUS)                                  \
  (1.0f - (SUS)) / (((DEC) <= 0.0f ? 1.0f : (DEC)) * (samplerate))
#define RELEASE_INCREMENT(samplerate, REL)                                     \
  (REL) <= 0.0f ? 1.0f : 1.0f / ((REL) * (samplerate))

#define NYQUIST(samplerate) (samplerate) / 2.0f

void stream_get(void *data, SDL_AudioStream *stream, i32 add, i32 total);

enum ENV_STATE : size_t { ATK, DEC, REL, SUS, OFF };

//    STATE_INTEGRATOR,
//    STATE_DC_X,
//    STATE_DC_Y,
enum OSC_STATE : size_t { ENVELOPE, PHASE, TIME, STATE_COUNT };

class LPF {
public:
  LPF(f32 cutoff, i32 sample_rate);
  void lerp(f32 target[SIZES::CHANNEL_MAX], i32 c);
  f32 get_alpha(void) const { return alpha; }
  f32 derive_alpha(f32 cutoff, i32 sample_rate);
  const f32 *get_array(void) const { return low; }
  f32 *get_array(void) { return low; }

private:
  f32 alpha;
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
  Oscilator(Oscilator_Cfg c) : cfg(c) {}

  f32 get_phase_val(void) const { return phase; }
  f32 get_time_val(void) const { return time; }
  void start(void);

  const Oscilator_Cfg get_cfg(void) const { return cfg; }

  void increment_phase(f32 inc, f32 max);
  void increment_time(f32 dt);
  f32 *get_sample_array(void) { return gen; }

private:
  f32 gen[SIZES::CHANNEL_MAX];
  Oscilator_Cfg cfg;

  f32 phase;
  f32 time;
};

class Voice {
public:
  Voice(f32 cutoff_low, i32 sample_rate);
  bool done(void) const;
  bool releasing(void) const;
  const std::vector<Oscilator> &get_osc_array(void) const { return oscs; }
  std::vector<Oscilator> &get_osc_array(void) { return oscs; }
  size_t get_osc_count(void) const { return oscs.size(); }
  i32 get_key(void) const { return midi_key; }
  void set_key(i32 key) { midi_key = key; }
  f32 get_freq(void) const { return freq; }
  void set_freq(f32 val) { freq = val; }
  i32 get_active_count(void) const { return active_oscilators; }
  void set_active_count(i32 val) { active_oscilators = val; }
  LPF &get_lpf(void) { return lpf; }

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
  Wave_Table(i32 sample_rate, size_t table_size, f32 duty_cycle);
  void re_generate(i32 sample_rate, size_t table_size, f32 duty_cycle);
  void generate(i32 sample_rate, f32 duty_cycle);
  size_t index_octave(f32 freq) const;
  size_t get_size(void) const { return size; }
  const f32 *get_table(size_t id, size_t index) const;

  void sine(f32 buf[SIZES::MAX_TABLE_SIZE], size_t N);
  void fourier_saw(f32 buf[SIZES::MAX_TABLE_SIZE], size_t N, size_t harm);
  void fourier_square(f32 buf[SIZES::MAX_TABLE_SIZE], size_t N, size_t harm);
  void fourier_triangle(f32 buf[SIZES::MAX_TABLE_SIZE], size_t N, size_t harm);
  void fourier_pulse(f32 buf[SIZES::MAX_TABLE_SIZE], size_t N, size_t harm,
                     f32 duty_cycle);

private:
  f32 tables[WAVEFORM_TYPE::WAVEFORM_COUNT][SIZES::OCTAVES]
            [SIZES::MAX_TABLE_SIZE];
  f32 freq_mapper[SIZES::OCTAVES];
  size_t size;

  void fourier_saw(void);
  void sine(void);
  void fourier_square(void);
};

class Synth {
public:
  Synth(void);
  void set_cfg(Synth_Cfg c) { cfg = c; }

  const Synth_Cfg &get_cfg(void) const { return cfg; }
  const Wave_Table &get_wave_table(void) const { return wave_table; }
  Wave_Table &get_wave_table(void) { return wave_table; }
  const std::vector<Voice> &get_voices(void) const { return voices; }
  std::vector<Voice> &get_voices(void) { return voices; }

  void new_oscilators(std::vector<Oscilator_Cfg> osc_cfgs);
  void loop_voicings_off(i32 midi_key);
  void loop_voicings_on(i32 midi_key);

private:
  Synth_Cfg cfg;
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
