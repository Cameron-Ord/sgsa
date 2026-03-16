#ifndef TYPEDEF_HPP
#define TYPEDEF_HPP
#include <cstdint>
#include <string>
typedef float f32;

typedef int64_t i64;
typedef uint64_t u64;
typedef int32_t i32;
typedef uint32_t u32;
typedef int16_t i16;
typedef uint16_t u16;
typedef int8_t i8;
typedef uint8_t u8;

#define PI 3.141592653589793f
#define NYQUIST(samplerate) (samplerate) / 2.0f

enum LFOS : i32 { LFO_1, LFO_2, LFO_COUNT };

enum WAVE_FORMS : size_t {
  SAW = 0,
  SINE = 1,
  SQUARE = 2,
  TRIANGLE = 3,
  PULSE = 4,
  WAVEFORM_COUNT,
};

enum CONSTANTS : size_t {
  VOICES = 16,
  CONTROLLER_NAME_MAX = 256,
  CHANNEL_MAX = 2,
  MAX_OSC_COUNT = 6,
};

enum SYNTH_PARAMETER : size_t {
  S_ATTACK,
  S_DECAY,
  S_SUSTAIN,
  S_RELEASE,
  S_VOLUME,
  S_GAIN,
  S_LOW_PASS,
  S_TREMOLO_DEPTH,
  S_DELAY_TIME,
  S_BPM,
  S_PARAM_COUNT
};

enum NOTE_DURATIONS : size_t {
  D_WHOLE,
  D_HALF,
  D_QUARTER,
  D_EIGHT,
  D_SIXTEENTH,
  D_COUNT
};

enum MIDI_EVENT_CONSTANTS : size_t {
  NOTE_ON = 0x90,
  NOTE_OFF = 0x80,
  PITCH_BEND = 0xE0,
  CONTROL = 0xB0,
  CONTROL_ON = 0x7F,
  CONTROL_OFF = 0x0,
  CONTROL_MOD_WHEEL = 1,
  CONTROL_VOLUME_KNOB = 7,
  INPUT_BUFFER_MAX = 64
};

struct ParamF32 {
  ParamF32(void) : name(), min(0.0f), max(0.0f), value(0.0f) {}
  ParamF32(std::string _name, f32 _min, f32 _max, f32 _value)
      : name(_name), min(_min), max(_max), value(_value) {}
  std::string name;
  f32 min, max;
  f32 value;
};

#endif
