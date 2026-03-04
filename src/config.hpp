#ifndef CONFIG_H
#define CONFIG_H
#include "typedef.hpp"
#include <string>
bool parse_config(void);  

enum ENV_TYPE : size_t {
  ADSR,
  AR,
};

enum LFO_TYPE : size_t {
  VIBRATO,
  TREMOLO,
};

enum WAVEFORM_TYPE : size_t {
    SAW,
    SINE,
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
  inline constexpr LFO_TYPE LFO_MODE = LFO_TYPE::TREMOLO;

  inline constexpr f32 ATK = 0.1f;
  inline constexpr f32 DEC = 0.1f;
  inline constexpr f32 SUS = 0.1f;
  inline constexpr f32 REL = 0.1f;
  inline constexpr ENV_TYPE ENV_TYPE = ENV_TYPE::ADSR;

  inline constexpr i32 CHANNELS = SIZES::CHANNEL_MAX;
  inline constexpr i32 SAMPLE_RATE = 48000;
  inline constexpr size_t VOICINGS = SIZES::VOICES;
  inline constexpr size_t WT_SIZE = SIZES::MAX_TABLE_SIZE;
  inline constexpr f32 TEMPO = 120.0f;
  inline constexpr f32 NOTE_DURATION = 1.0f;
  inline constexpr f32 MAIN_VOLUME = 1.0f;
  inline constexpr f32 GAIN_LEVEL = 3.0f;

  inline constexpr f32 FILTER_UNSET = 1.0f;

  inline constexpr f32 DUTY_CYCLE = 0.5f;
  inline constexpr f32 DETUNE = 1.0f;
  inline constexpr f32 OSC_VOLUME = 1.0f;
  inline constexpr f32 STEP = 1.0f;
  inline constexpr WAVEFORM_TYPE WAVEFORM = WAVEFORM_TYPE::SAW;
  inline constexpr bool USE_FILTER = false;
}

struct Oscilator_Cfg{
  f32 cycle = DEFAULTS::DUTY_CYCLE;
  f32 detune = DEFAULTS::DETUNE;
  f32 volume = DEFAULTS::OSC_VOLUME;
  f32 step = DEFAULTS::STEP;
  size_t table_id = DEFAULTS::WAVEFORM;
};

struct Lfo_Params {
  bool on = DEFAULTS::LFO_ON;
  f32 rate = DEFAULTS::LFO_RATE;
  f32 depth = DEFAULTS::LFO_DEPTH;
  f32 timer = DEFAULTS::LFO_TIMER;
  LFO_TYPE mode = DEFAULTS::LFO_MODE;
};

struct Env_Params {
  f32 attack = DEFAULTS::ATK;
  f32 decay = DEFAULTS::DEC;
  f32 sustain = DEFAULTS::SUS;
  f32 release = DEFAULTS::REL;
  ENV_TYPE env_id = DEFAULTS::ENV_TYPE;
};

struct Audio_Params {
  Audio_Params(void);
  Audio_Params(i32 CHANNELS, i32 SAMPLE_RATE, size_t VOICE_COUNT, 
    size_t WT_SIZE, f32 TEMPO, f32 NOTE_DUR, f32 CUTOFF_LOW, 
    f32 CUTOFF_HIGH, f32 VOLUME, f32 GAIN);

  f32 volume = DEFAULTS::MAIN_VOLUME;
  f32 gain = DEFAULTS::GAIN_LEVEL;
  i32 channels = DEFAULTS::CHANNELS;
  i32 sample_rate = DEFAULTS::SAMPLE_RATE;
  size_t voicings = DEFAULTS::VOICINGS;
  size_t wave_table_size = DEFAULTS::WT_SIZE;
  f32 tempo = DEFAULTS::TEMPO;
  f32 note_duration = DEFAULTS::NOTE_DURATION;
  f32 lpf_alpha_low = DEFAULTS::FILTER_UNSET;
  f32 lpf_alpha_high = DEFAULTS::FILTER_UNSET;
  bool use_filter = DEFAULTS::USE_FILTER;
};

#endif
