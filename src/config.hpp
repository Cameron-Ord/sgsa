#ifndef CONFIG_H
#define CONFIG_H
#include "typedef.hpp"

enum LFO_TYPE : i32 {
  VIBRATO,
  TREMOLO,
};

enum WAVEFORM_TYPE : size_t {
  SAW,
  SINE,
  SQUARE,
  TRIANGLE,
  PULSE,
  WAVEFORM_COUNT,
};

enum SIZES : size_t {
  VOICES = 12,
  CONTROLLER_NAME_MAX = 256,
  CHANNEL_MAX = 2,
  MAX_TABLE_SIZE = 4096,
  OCTAVES = 12,
};

namespace DEFAULTS {
inline constexpr bool LFO_ON = false;
inline constexpr f32 LFO_RATE = 2.0f;
inline constexpr f32 LFO_DEPTH = 1.0f;
inline constexpr f32 LFO_TIMER = 0.33f;
inline constexpr i32 LFO_MODE = LFO_TYPE::TREMOLO;

inline constexpr f32 ATK = 0.3f;
inline constexpr f32 DEC = 0.7f;
inline constexpr f32 SUS = 0.0f;
inline constexpr f32 REL = 0.8f;

inline constexpr i32 CHANNELS = SIZES::CHANNEL_MAX;
inline constexpr i32 SAMPLE_RATE = 48000;
inline constexpr i32 VOICINGS = SIZES::VOICES;
inline constexpr i32 WT_SIZE = SIZES::MAX_TABLE_SIZE;
inline constexpr f32 TEMPO = 120.0f;
inline constexpr f32 NOTE_DURATION = 1.0f;
inline constexpr f32 MAIN_VOLUME = 1.0f;
inline constexpr f32 GAIN_LEVEL = 3.0f;
inline constexpr f32 FILTER_CUTOFF_HIGH = (f32)SAMPLE_RATE / 2.0f;
inline constexpr f32 FILTER_CUTOFF_LOW = 40.0f;
inline constexpr f32 FILTER_UNSET = 1.0f;

inline constexpr f32 DUTY_CYCLE = 0.5f;
inline constexpr f32 DETUNE = 1.0f;
inline constexpr f32 OSC_VOLUME = 1.0f;
inline constexpr f32 STEP = 1.0f;
inline constexpr i32 WAVEFORM = WAVEFORM_TYPE::SAW;
inline constexpr bool USE_FILTER = false;
} // namespace DEFAULTS

struct Synth_Cfg {
  bool lfo_on = DEFAULTS::LFO_ON;
  bool use_filter = DEFAULTS::USE_FILTER;

  f32 lfo_rate = DEFAULTS::LFO_RATE;
  f32 lfo_depth = DEFAULTS::LFO_DEPTH;
  f32 lfo_timer = DEFAULTS::LFO_TIMER;
  f32 volume = DEFAULTS::MAIN_VOLUME;
  f32 gain = DEFAULTS::GAIN_LEVEL;
  f32 tempo = DEFAULTS::TEMPO;
  f32 note_duration = DEFAULTS::NOTE_DURATION;
  f32 filter_cutoff_low = DEFAULTS::FILTER_CUTOFF_LOW;
  f32 filter_cutoff_high = DEFAULTS::FILTER_CUTOFF_HIGH;
  f32 env_attack = DEFAULTS::ATK;
  f32 env_decay = DEFAULTS::DEC;
  f32 env_sustain = DEFAULTS::SUS;
  f32 env_release = DEFAULTS::REL;

  i32 channels = DEFAULTS::CHANNELS;
  i32 sample_rate = DEFAULTS::SAMPLE_RATE;
  i32 lfo_mode = DEFAULTS::LFO_MODE;
  i32 voicings = DEFAULTS::VOICINGS;
  i32 wave_table_size = DEFAULTS::WT_SIZE;

  f32 duty_cycle = DEFAULTS::DUTY_CYCLE;

  void print(void) const;
};

struct Oscilator_Cfg {
  f32 detune = DEFAULTS::DETUNE;
  f32 volume = DEFAULTS::OSC_VOLUME;
  f32 octave_step = DEFAULTS::STEP;
  i32 waveform = DEFAULTS::WAVEFORM;

  void print(void) const;
};

#endif
