#include "sgsa.hpp"
#include <toml++/toml.hpp>
#include <iostream>

bool parse_config(void){  
  toml::table tbl;
  try {
    tbl = toml::parse_file("config.toml");
    std::cout << tbl << std::endl;
  } catch (const toml::parse_error& err) {
    std::cerr << "Config parsing failed:\n" << err << std::endl;
    return false;
  }
  return true;
}


Params::Params(void) : lfop(), envp(), ap() {}

Oscilator_Cfg::Oscilator_Cfg(void){
  const f32 CYCLE = 0.5f;
  const std::string DEFAULT_OSC = "saw";
  const size_t TABLE_ID = TABLE_SAW;
  const f32 DETUNE = 1.0f, VOLUME = 1.0f, STEP = 1.0f;

  cycle = CYCLE;
  table_id = TABLE_ID;
  name = DEFAULT_OSC;
  detune = DETUNE, volume = VOLUME, step = STEP;
}

Lfo_Params::Lfo_Params(void){
  const f32 RATE = 2.0f, DEPTH = 1.0f, TIMER = 0.33f;
  const u8 MODE = MODE_VIBRATO;
  rate = RATE, depth = DEPTH, timer = TIMER;
  mode = MODE;
}

Audio_Params::Audio_Params(void){
  const i32 CHANNELS = 2, SAMPLE_RATE = 48000;
  const i32 VOICINGS = 8, WAVE_TABLE_SIZE = 512;
  const f32 TEMPO = 120.0f, NOTE_DURATION = 1.0f;
  channels = CHANNELS, sample_rate = SAMPLE_RATE;
  voicings = VOICINGS, wave_table_size = WAVE_TABLE_SIZE;
  tempo = TEMPO, note_duration = NOTE_DURATION;
  
  const f32 LOW_CUTOFF_FREQ = 80.0f;
  const f32 DT = 1.0f / SAMPLE_RATE;
  const f32 RC_HIGH = 1.0f / (2.0f * PI * NYQUIST((f32)SAMPLE_RATE));
  const f32 RC_LOW = 1.0f / (2.0f * PI * LOW_CUTOFF_FREQ);

  const f32 ALPHA_HIGH = DT / (RC_HIGH + DT);
  const f32 ALPHA_LOW = DT / (RC_LOW + DT);

  lpf_alpha_high = ALPHA_HIGH;
  lpf_alpha_low = ALPHA_LOW;
}

Env_Params::Env_Params(void){
  const std::string TYPE = "adsr";
  const f32 ATK = 0.2f, DEC = 0.3f, SUS = 0.75f, REL = 0.25f;
  attack = ATK, decay = DEC, sustain = SUS, release = REL;
  type = TYPE;
}
