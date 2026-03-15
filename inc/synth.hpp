#ifndef AUDIO_HPP
#define AUDIO_HPP
#include "typedef.hpp"

#include <vector>
#include <array>
#include <string>

#include <portmidi.h>

#define CENTS_TO_OCTAVE 1.0f / 1200.0f
#define ONE_SEMITONE_CENTS 100.0f
#define TWO_SEMITONE_CENTS 200.0f

f32 exp_hard_clip(f32 sample, f32 gain, f32 mix);
f32 polynomial_soft_clip(f32 sample, f32 gain);

#define PI 3.141592653589793f
// (VALUE - VALUE) / SAMPLES
#define ATTACK_INCREMENT(samplerate, ATK)                                      \
  1.0f / (((ATK) <= 0.0f ? 1.0f : (ATK)) * (samplerate))
#define DECAY_INCREMENT(samplerate, DEC, SUS)                                  \
  (1.0f - (SUS)) / (((DEC) <= 0.0f ? 1.0f : (DEC)) * (samplerate))
#define RELEASE_INCREMENT(samplerate, REL)                                     \
  (REL) <= 0.0f ? 1.0f : 1.0f / ((REL) * (samplerate))

#define NYQUIST(samplerate) (samplerate) / 2.0f

enum LFO_ACCESS : i32 {
  LFO_1,
  LFO_2,
  LFO_COUNT
};

enum WAVEFORM_TYPE : size_t {
  SAW = 0,
  SINE = 1,
  SQUARE = 2,
  TRIANGLE = 3,
  PULSE = 4,
  WAVEFORM_COUNT,
};

enum SIZES : size_t {
  VOICES = 16,
  CONTROLLER_NAME_MAX = 256,
  CHANNEL_MAX = 2,
  MAX_OSC_COUNT = 6,
};

enum EVENT_CONSTANTS : size_t {
  INPUT_BUFFER_MAX = 64
};

enum EVENT_TYPE : size_t {
  NOTE_ON = 0x90,
  NOTE_OFF = 0x80,
  PITCH_BEND = 0xE0,
  CONTROL = 0xB0,
  CONTROL_ON = 0x7F,
  CONTROL_OFF = 0x0,
  CONTROL_MOD_WHEEL = 1,
  CONTROL_VOLUME_KNOB = 7,
};

struct Midi_Input_Msg {
  Midi_Input_Msg(void) : status(0), msg1(0), msg2(0) {}
  Midi_Input_Msg(u32 _status, u32 _msg1, u32 _msg2) : status(_status), msg1(_msg1), msg2(_msg2) {}
  u32 status;
  u32 msg1, msg2;
};

struct Keyboard_Command {
  enum Type : i32 {
    pitch_bend,
    note_on,
    note_off,
    mod_wheel,
    vol_knob,
  } type;

  Midi_Input_Msg input;
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
  i32 read_input(void);
  void clear_msg_buf(void);
  const PmEvent *get_event_at(i32 pos) const;
  const std::array<PmEvent, INPUT_BUFFER_MAX>& get_input_buffer(void) const { return input_buffer; }
  Midi_Input_Msg parse_event(PmEvent event);
  
private:
  std::string input_name;
  i32 input_id;
  PortMidiStream *stream;
  std::array<PmEvent, INPUT_BUFFER_MAX> input_buffer;
};


enum ENV_STATE : size_t { ATK, DEC, REL, SUS, OFF };
//    STATE_INTEGRATOR,
//    STATE_DC_X,
//    STATE_DC_Y,

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

class Lfo {
public:
  Lfo(void) : active(true), phase(0.0f) {}
  void increment(f32 rate, i32 sample_rate);
  void set_active(bool val) { active = val; }
  bool get_active_state(void) { return active; }
  f32 lfo_sine(void);
private:
  bool active;
  f32 phase;
};

class Amp_Modulator {
public:
  Amp_Modulator(void) : lfo() {}
  Lfo& get_lfo(void) { return lfo; }
private:
  Lfo lfo;
};

class Freq_Modulator {
public:
  Freq_Modulator(void) : lfo() {}
  
  f32 create_vibrato(f32 sine, f32 cents) const;

  Lfo& get_lfo(void) { return lfo; }
private:
  Lfo lfo;
};

class LPF {
public:
  LPF(void);
  void reset(void);
  void lerp(const std::array<f32, CHANNEL_MAX>& target, size_t c, i32 sample_rate, f32 cutoff);
  f32 alpha(f32 cutoff, i32 sample_rate);
 
  std::array<f32, CHANNEL_MAX>& get_array(void) { return low; }
  const std::array<f32, CHANNEL_MAX>& get_array(void) const { return low; }
  f32 get_value_at(size_t pos) const;

private:
  std::array<f32, CHANNEL_MAX> low;
};

class Oscillator {
public:
  Oscillator(void);
  void reset(size_t voice_index);
  
  f32 phase_clamp(f32 phase_val, f32 max);
  void increment_phase_at(f32 inc, f32 max, size_t pos);
  void increment_time_at(f32 dt, size_t pos);
 
  std::array<f32, CHANNEL_MAX>* get_sample_array_at(size_t pos);
  f32 get_sample_at(const std::array<f32, CHANNEL_MAX>*buf, size_t pos) const;
  void set_sample_at(std::array<f32, CHANNEL_MAX>*buf, size_t pos, f32 value); 

  f32 get_inc_at(size_t pos) const;
  f32 get_phase_at(size_t pos) const;
  f32 get_time_at(size_t pos) const;

  f32 get_detune(void) const { return detune; }
  f32 get_duty(void) const { return duty; }
  i32 get_waveform(void) const { return waveform; }

  void set_detune(f32 val) { detune = val; }
  void set_duty(f32 val) { duty = val; }
  void set_waveform(i32 val) { waveform = val; }

private:
  f32 detune = 1.0f, detune_min = 0.9f, detune_max = 1.1f;
  f32 duty = 0.5f, duty_min = 0.1f, duty_max = 1.0f;
  i32 waveform = SAW;

  std::array<std::array<f32, CHANNEL_MAX>, VOICES> gen;
  std::array<f32, VOICES> phase;
  std::array<f32, VOICES> time;
};

class Voice {
public:
  Voice(void);
  LPF &get_lpf(void) { return lpf; }

  u32 get_key(void) const { return midi_key; }
  i32 get_active_count(void) const { return active_oscillators; }
  u8 get_env_state(void) const { return env_state; }
 
  f32 get_envelope(void) const { return envelope; }
  f32 get_freq(void) const { return freq; }

  std::array<f32, CHANNEL_MAX>& get_sum_array(void) { return voice_sums; }
  std::array<f32, CHANNEL_MAX>& get_clipped_array(void) { return clipped_sums; }
  std::array<f32, CHANNEL_MAX>& get_out_array(void) { return voice_out; }
  std::array<f32, CHANNEL_MAX>& get_filtered_array(void) { return filtered_sums; }

  void add_sum_at(size_t pos, f32 sample);
  void set_clipped_at(size_t pos, f32 val);
  void set_filtered_at(size_t pos, f32 val);
  void set_out_at(size_t pos, f32 val);

  f32 get_sum_at(size_t pos) const;
  f32 get_clipped_at(size_t pos) const;
  f32 get_filtered_at(size_t pos) const;
  f32 get_out_at(size_t pos) const;
 
  bool done(void) const;
  bool releasing(void) const;

  void zero_voice_sums(void);
  void set_key(u32 key) { midi_key = key; }
  void set_freq(f32 val) { freq = val; }
  void set_active_count(i32 val) { active_oscillators = val; }
  void set_envelope(f32 val) { envelope = val; }
  void set_env_state(u8 state) { env_state = state; }
  void ar(i32 samplerate, f32 atk, f32 rel);
  void adsr(i32 samplerate, f32 atk, f32 dec, f32 sus, f32 rel);

  Amp_Modulator& get_amod(void) { return amod; }
  Freq_Modulator& get_fmod(void) { return fmod; }

  void set_vol_mult(f32 val) { volume_multiplier = val; }
  f32 get_vol_mult(void) { return volume_multiplier; }

private:
  i32 active_oscillators;
  u32 midi_key;
  u8 env_state;
  f32 freq;
  f32 envelope;
  Freq_Modulator fmod;
  Amp_Modulator amod;
  f32 volume_multiplier;
  LPF lpf;

  std::array<f32, CHANNEL_MAX> voice_sums;
  std::array<f32, CHANNEL_MAX> clipped_sums;
  std::array<f32, CHANNEL_MAX> filtered_sums;
  std::array<f32, CHANNEL_MAX> voice_out;
};


class Generator {
public:
  Generator(void) = default;
  f32 polyblep(f32 inc, f32 phase);
  f32 poly_square(f32 inc, f32 phase, f32 duty);
  f32 poly_saw(f32 inc, f32 phase);
  f32 square(f32 phase, f32 duty);
  f32 sawtooth(f32 phase);
private:
};

class Synth {
public:
  Synth(void);

  Delay& get_delay(void) { return delay; }
  Generator& get_generator(void) { return generator; }
  
  std::array<f32, CHANNEL_MAX>& get_sum_array(void) { return loop_sums; }
  std::array<Voice, VOICES> &get_voices(void) { return voices; }
  const std::array<Voice, VOICES> &get_voices(void) const { return voices; }
 
  std::vector<Oscillator>& get_oscillators(void) { return oscs; }
  Oscillator *get_osc_at(size_t pos);
 
  void zero_loop_sums(void);
  void add_sum_at(size_t pos, f32 sum);
  f32 get_sum_at(size_t pos) const;
  
  void loop_voicings_off(u32 midi_key);
  void loop_voicings_on(u32 midi_key, f32 norm_velocity);

   const f32& get_attack(void)  const { return attack; }
   const f32& get_decay(void)  const { return decay; }
   const f32& get_sustain(void) const  { return sustain; }
   const f32& get_release(void)  const { return release; }
   const f32& get_attack_max(void) const  { return attack_max; }
   const f32& get_decay_max(void)  const { return decay_max; }
   const f32& get_sustain_max(void) const  { return sustain_max; }
   const f32& get_release_max(void) const  { return release_max; }
   const f32& get_attack_min(void)  const { return attack_min; }
   const f32& get_decay_min(void)  const { return decay_min; }
   const f32& get_sustain_min(void) const  { return sustain_min; }
   const f32& get_release_min(void)  const { return release_min; }
   const f32& get_volume(void)  const { return volume; }
   const f32& get_volume_min(void)  const { return volume_min; }
   const f32& get_volume_max(void)  const { return volume_max; }
   const f32& get_gain(void)  const { return gain; }
   const f32& get_low_pass(void)  const { return low_pass_hz; }
   const i32& get_channels(void) const  { return channels; }
   const i32& get_sample_rate(void)  const { return sample_rate; }
   const f32& get_low_pass_max(void)  const { return low_pass_hz_max; }
   const f32& get_low_pass_min(void) const  { return low_pass_hz_min; }
   const f32& get_gain_max(void) const  { return gain_max; }
   const f32& get_gain_min(void) const  { return gain_min; }
   const f32& get_trem_rate(void) const  { return tremolo_rate; } 
   const f32& get_trem_depth(void) const  { return tremolo_depth; } 
   const f32& get_trem_depth_min(void)  const { return trem_depth_min; } 
   const f32& get_trem_depth_max(void)  const { return trem_depth_max; } 
   const f32& get_vibrato_depth(void) const  { return vibrato_depth; }
   const f32& get_vibrato_rate(void) const  { return vibrato_rate; }
   const f32& get_pitch_bend(void) const  { return pitch_bend; }
   
  void set_attack(f32 val) { attack = val; }
  void set_decay(f32 val) { decay = val; }
  void set_sustain(f32 val) { sustain = val; }
  void set_release(f32 val) { release = val; }
  void set_pitch_bend(f32 val) { pitch_bend = val; }
  void set_vibrato_depth(f32 val) { vibrato_depth = val; }
  void set_trem_depth(f32 val) { tremolo_depth = val; }
  void set_volume(f32 val) { volume = val; }
  void set_gain(f32 val) { gain = val; }
  void set_low_pass(f32 val) { low_pass_hz = val; }
  
  f32 calculate_pitch_bend(f32 cents, f32 normalized_event) const;
  f32 map_vibrato_depth(f32 normalized_event) const;

  std::vector<Keyboard_Command> read_event(Controller& cont);
  void run_events(std::vector<Keyboard_Command>& commands);

private:
  f32 volume = 1.0f, volume_min = 0.0f, volume_max = 2.0f;
  f32 gain = 1.0f, gain_min = 0.5f, gain_max = 16.0f;
  f32 low_pass_hz = 2000.0f, low_pass_hz_min = 200.0f, low_pass_hz_max = 20000.0f;
  i32 channels = 2, channel_max = 2;
  i32 sample_rate = 48000, sample_rate_max = 96000;

  f32 attack = 0.2f, attack_min = 0.0f, attack_max = 2.5f;
  f32 decay = 0.2f, decay_min = 0.0f, decay_max = 2.5f;
  f32 sustain = 0.4f, sustain_min = 0.0f, sustain_max = 2.5f;
  f32 release = 0.2f, release_min = 0.0f, release_max = 2.5f;

  f32 pitch_bend = 1.0f;
  f32 vibrato_rate = 5.0f, vibrato_rate_max = 15.0f;
  f32 vibrato_depth = 0.0f, vibrato_max = 50.0f;
  
  f32 tremolo_rate = 2.0f, trem_rate_max = 16.0f;
  f32 tremolo_depth = 0.1f, trem_depth_min = 0.1f, trem_depth_max = 1.0f;

  std::vector<Oscillator> oscs;
  std::array<Voice, VOICES> voices;
  Generator generator;
  Delay delay;
  std::array<f32, CHANNEL_MAX> loop_sums;
};

#endif
