#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "config.hpp"
#include "typedef.hpp"

#include <vector>
#include <array>

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

struct Waveform_Vec4f {
  size_t waveforms, oscillators, octaves, samples;
  std::vector<f32> data;

  Waveform_Vec4f(size_t _waveforms, size_t _oscillators, size_t _octaves, size_t _samples);
  bool set_at(size_t wave_p, size_t osc_p, size_t oct_p, size_t sample_p, f32 val);
  bool valid(size_t pos) const;
  const f32 *get_at(size_t wave_p, size_t osc_p, size_t oct_p, size_t sample_p) const;
  size_t index(size_t wave_p, size_t osc_p, size_t oct_p, size_t sample_p) const;
};

class Delay {
public:
  Delay(i32 sample_rate, f32 delay_time_s, f32 _feedback);
  void delay_write(f32 sample);
  f32 delay_read(void);
  void increment(size_t read_inc, size_t write_inc);
private:
  std::vector<f32> buffer;
  size_t read, write;
  size_t start, end;
  f32 feedback;
};

class LPF {
public:
  LPF(f32 cutoff, i32 sample_rate);
  void reset(void);
  void set_alpha(f32 val)  { alpha = val; }
  void lerp(const std::array<f32, CHANNEL_MAX>& target, size_t c);
 
  f32 get_alpha(void) const { return alpha; }
  f32 derive_alpha(f32 cutoff, i32 sample_rate);
  std::array<f32, CHANNEL_MAX>& get_array(void) { return low; }
  const std::array<f32, CHANNEL_MAX>& get_array(void) const { return low; }
  f32 get_value_at(size_t pos) const;

private:
  f32 alpha;
  std::array<f32, CHANNEL_MAX> low;
};

class Lfo {
public:
  Lfo(void) = default;
  void increment_lfo(f32 inc);
private:
  f32 phase;
};

class Oscillator {
public:
  Oscillator(void);

  f32 get_phase_val(void) const { return phase; }
  f32 get_time_val(void) const { return time; }
  std::array<f32, CHANNEL_MAX>& get_sample_array(void) { return gen; }
  f32 get_sample_at(size_t pos) const;
  
  void mult_sample_at(size_t pos, f32 factor);
  void set_sample_at(size_t pos, f32 value); 
  void start(void);
  void increment_phase(f32 inc, f32 max);
  void increment_time(f32 dt);

private:
  std::array<f32, CHANNEL_MAX> gen;
  f32 phase;
  f32 time;
};

class Voice {
public:
  Voice(f32 cutoff_low, i32 sample_rate, size_t osc_count, size_t lfo_count);
  LPF &get_lpf(void) { return lpf; }

  i32 get_key(void) const { return midi_key; }
  i32 get_active_count(void) const { return active_oscillators; }
  u8 get_env_state(void) const { return env_state; }
 
  f32 get_envelope(void) const { return envelope; }
  f32 get_freq(void) const { return freq; }
  std::array<f32, CHANNEL_MAX>& get_sum_array(void) { return voice_sums; }

  std::vector<Oscillator> &get_osc_array(void) { return oscs; }
  const std::vector<Oscillator> &get_osc_array(void) const { return oscs; }
  Oscillator* get_osc_at(size_t pos);
  f32 get_sum_at(size_t pos) const;
 
  bool done(void) const;
  bool releasing(void) const;

  void add_sum_at(size_t pos, f32 sample);
  void zero_voice_sums(void);
  void set_key(i32 key) { midi_key = key; }
  void set_freq(f32 val) { freq = val; }
  void set_active_count(i32 val) { active_oscillators = val; }
  void set_envelope(f32 val) { envelope = val; }
  void set_env_state(u8 state) { env_state = state; }
  void ar(i32 samplerate, f32 atk, f32 rel);
  void adsr(i32 samplerate, f32 atk, f32 dec, f32 sus, f32 rel);

private:
  i32 active_oscillators;
  i32 midi_key;
  u8 env_state;
  f32 freq;
  f32 envelope;
  std::vector<Oscillator> oscs;
  std::vector<Lfo> lfos;
  std::array<f32, CHANNEL_MAX> voice_sums;
  LPF lpf;
};

class Wave_Table {
public:
  Wave_Table(Synth_Cfg scfg, std::vector<Oscillator_Cfg> ocfgs);

  size_t index_octave(f32 freq) const;
  const Waveform_Vec4f* get_table(void) const { return &table; }

  void generate(Synth_Cfg scfg, std::vector<Oscillator_Cfg> ocfgs);
 
  void sine(Waveform_Vec4f& v, size_t osc, size_t octave, size_t N);
  void fourier_saw(Waveform_Vec4f& v, size_t osc, size_t octave, size_t N, size_t harm);
  void fourier_square(Waveform_Vec4f& v, size_t osc, size_t octave, size_t N, size_t harm);
  void fourier_triangle(Waveform_Vec4f& v, size_t osc, size_t octave, size_t N, size_t harm);
  void fourier_pulse(Waveform_Vec4f& v, size_t osc, size_t octave, size_t N, size_t harm, f32 duty_cycle);
private:
  Waveform_Vec4f table;
  std::vector<f32> freq_range;
};

class Synth {
public:
  Synth(void);
  void zero_loop_sums(void);
  std::array<f32, CHANNEL_MAX>& get_sum_array(void) { return loop_sums; }
  void add_sum_at(size_t pos, f32 sum);
  f32 get_sum_at(size_t pos);

  const Synth_Cfg &get_synth_cfg(void) const { return synth_cfg; }
  const Envelope_Cfg &get_env_cfg(void) const { return env_cfg; }
  
  const std::vector<Lfo_Cfg> &get_lfo_cfgs(void) { return lfo_cfgs; }
  const std::vector<Oscillator_Cfg>& get_osc_cfgs(void) const { return osc_cfgs; }
  const std::vector<Voice> &get_voices(void) const { return voices; }
  std::vector<Voice> &get_voices(void) { return voices; }
 
  Wave_Table &get_wave_table(void) { return wave_table; }
  const Wave_Table &get_wave_table(void) const { return wave_table; }

  const Oscillator_Cfg *get_osc_cfg_at(size_t pos) const;
  size_t get_osc_count(void) const { return osc_cfgs.size(); }
  Delay& get_delay(void) { return delay; }
  
  void set_cfg(Synth_Cfg c) { synth_cfg = c; }
  void set_osc_cfgs(std::vector<Oscillator_Cfg> ocfgs){ osc_cfgs = ocfgs; }
  void update_lpf(void);
  void new_Oscillators(std::vector<Oscillator_Cfg> osc_cfgs);
  void loop_voicings_off(i32 midi_key);
  void loop_voicings_on(i32 midi_key);

private:
  Synth_Cfg synth_cfg;
  Envelope_Cfg env_cfg;
  std::vector<Oscillator_Cfg> osc_cfgs;
  std::vector<Lfo_Cfg> lfo_cfgs;
  std::vector<Voice> voices;
  Wave_Table wave_table;
  Delay delay;
  std::array<f32, CHANNEL_MAX> loop_sums;
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
