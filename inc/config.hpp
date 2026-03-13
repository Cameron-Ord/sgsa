#ifndef CONFIG_H
#define CONFIG_H
#include "typedef.hpp"

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
  VOICES = 12,
  CONTROLLER_NAME_MAX = 256,
  CHANNEL_MAX = 2,
  MAX_OSC_COUNT = 8,
};

namespace DEFAULTS {
inline constexpr bool LFO_ON = false;
inline constexpr f32 LFO_RATE = 6.0f;

inline constexpr f32 ATK = 0.01f;
inline constexpr f32 DEC = 0.3f;
inline constexpr f32 SUS = 0.7f;
inline constexpr f32 REL = 0.01f;

inline constexpr i32 CHANNELS = SIZES::CHANNEL_MAX;
inline constexpr i32 SAMPLE_RATE = 48000;
inline constexpr i32 VOICINGS = SIZES::VOICES;
inline constexpr f32 MAIN_VOLUME = 1.0f;
inline constexpr f32 GAIN_LEVEL = 1.25f;
inline constexpr f32 LOW_PASS_CUTOFF = 8000.0f;

inline constexpr f32 DUTY_CYCLE = 0.5f;
inline constexpr f32 DETUNE = 1.0f;
inline constexpr f32 OSC_VOLUME = 1.0f;
inline constexpr i32 WAVEFORM = WAVEFORM_TYPE::SAW;
} // namespace DEFAULTS

struct Synth_Cfg {
  f32 volume = DEFAULTS::MAIN_VOLUME;
  f32 gain = DEFAULTS::GAIN_LEVEL;
  f32 low_pass_cutoff = DEFAULTS::LOW_PASS_CUTOFF;
  i32 channels = DEFAULTS::CHANNELS;
  i32 sample_rate = DEFAULTS::SAMPLE_RATE;
  size_t voicings = DEFAULTS::VOICINGS;
};

struct Envelope_Cfg {
  f32 env_attack = DEFAULTS::ATK;
  f32 env_decay = DEFAULTS::DEC;
  f32 env_sustain = DEFAULTS::SUS;
  f32 env_release = DEFAULTS::REL;
};

struct Lfo_Cfg {
  bool lfo_on = DEFAULTS::LFO_ON;
  f32 lfo_rate = DEFAULTS::LFO_RATE;
};

struct Oscillator_Cfg {
  f32 duty = DEFAULTS::DUTY_CYCLE;
  f32 detune = DEFAULTS::DETUNE;
  f32 volume = DEFAULTS::OSC_VOLUME;
  size_t waveform = DEFAULTS::WAVEFORM;
};

#endif
