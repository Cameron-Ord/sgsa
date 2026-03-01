#include "sgsa.hpp"
#include <toml++/toml.hpp>
#include <iostream>
#include <unordered_map>

const std::string DEFAULT_WAVE_STR = "SAW";
const size_t DEFAULT_WAVE_ID = TABLE_SAW;

const std::string DEFAULT_ENV_STR = "ADSR";
const size_t DEFAULT_ENV_ID = ENV_ADSR;

const std::string PIANO_ENV_STR = "PIANO";


const std::unordered_map<std::string, size_t> table_id_map = {
  { std::string(DEFAULT_WAVE_STR), DEFAULT_WAVE_ID },
};

const std::unordered_map<std::string, u8> env_id_map = {
  { std::string(DEFAULT_ENV_STR), DEFAULT_ENV_ID },
  { std::string(PIANO_ENV_STR), ENV_PIANO }
};

bool parse_config(void){  
  toml::table tbl;
  try {
    tbl = toml::parse_file("config.toml");
    std::cout << tbl << "\n";
  } catch (const toml::parse_error& err) {
    std::cerr << "Config parsing failed: " << err << "\n";
    return false;
  }
  return true;
}

Params::Params(void) : lfop(), envp(), ap() {}
Params::Params(Lfo_Params lfop_tmp, Env_Params envp_tmp, Audio_Params ap_tmp)
  : lfop(lfop_tmp), envp(envp_tmp), ap(ap_tmp) {}

Oscilator_Cfg::Oscilator_Cfg(void){
  const f32 CYCLE = 0.5f;
  const f32 DETUNE = 1.0f, VOLUME = 1.0f, STEP = 1.0f;

  cycle = CYCLE;
  table_id = DEFAULT_WAVE_ID;
  name = DEFAULT_WAVE_STR;
  detune = DETUNE, volume = VOLUME, step = STEP;
}

Oscilator_Cfg::Oscilator_Cfg(f32 CYCLE, std::string NAME, f32 DETUNE, f32 VOLUME, f32 STEP){
  cycle = CYCLE;
  table_id = DEFAULT_WAVE_ID;
  name = DEFAULT_WAVE_STR;
  detune = DETUNE, volume = VOLUME, step = STEP;

  auto it = table_id_map.find(NAME);
  if(it != table_id_map.end()){
    name = it->first;
    table_id = it->second;
  }
}

Lfo_Params::Lfo_Params(void){
  const f32 RATE = 2.0f, DEPTH = 1.0f, TIMER = 0.33f;
  const u8 MODE = MODE_VIBRATO;
  rate = RATE, depth = DEPTH, timer = TIMER;
  mode = MODE;
}

Lfo_Params::Lfo_Params(f32 RATE, f32 DEPTH, f32 TIMER, u8 MODE){
  rate = RATE, depth = DEPTH, timer = TIMER, mode = MODE;
}

Audio_Params::Audio_Params(void){
  const i32 CHANNELS = 2, SAMPLE_RATE = 48000;
  const i32 VOICINGS = 8, WAVE_TABLE_SIZE = 512;
  const f32 TEMPO = 120.0f, NOTE_DURATION = 1.0f;

  channels = CHANNELS, sample_rate = SAMPLE_RATE;
  voicings = VOICINGS, wave_table_size = WAVE_TABLE_SIZE;
  tempo = TEMPO, note_duration = NOTE_DURATION;
  
  const f32 LOW_CUTOFF_FREQ = 80.0f;
  const f32 DT = 1.0f / (f32)SAMPLE_RATE;
  const f32 RC_HIGH = 1.0f / (2.0f * PI * NYQUIST((f32)SAMPLE_RATE));
  const f32 RC_LOW = 1.0f / (2.0f * PI * LOW_CUTOFF_FREQ);

  const f32 ALPHA_HIGH = DT / (RC_HIGH + DT);
  const f32 ALPHA_LOW = DT / (RC_LOW + DT);

  lpf_alpha_high = ALPHA_HIGH;
  lpf_alpha_low = ALPHA_LOW;
}


Audio_Params::Audio_Params(i32 CHANNELS, i32 SAMPLE_RATE, size_t VOICE_COUNT, 
    size_t WT_SIZE, f32 TEMPO, f32 NOTE_DUR, f32 CUTOFF_LOW, f32 CUTOFF_HIGH){
  channels = CHANNELS, sample_rate = SAMPLE_RATE;
  voicings = VOICE_COUNT, wave_table_size = WT_SIZE;
  tempo = TEMPO, note_duration = NOTE_DUR;

  const f32 DT = 1.0f / (f32)SAMPLE_RATE;
  const f32 RC_HIGH = 1.0f / (2.0f * PI * CUTOFF_HIGH);
  const f32 RC_LOW = 1.0f / (2.0f * PI * CUTOFF_LOW);

  const f32 ALPHA_HIGH = DT / (RC_HIGH + DT);
  const f32 ALPHA_LOW = DT / (RC_LOW + DT);

  lpf_alpha_high = ALPHA_HIGH;
  lpf_alpha_low = ALPHA_LOW;
}

Env_Params::Env_Params(void){
  const f32 ATK = 0.2f, DEC = 0.3f, SUS = 0.75f, REL = 0.25f;
  attack = ATK, decay = DEC, sustain = SUS, release = REL;
  type = DEFAULT_ENV_STR;
  env_id = DEFAULT_ENV_ID;
}

Env_Params::Env_Params(f32 ATK, f32 DEC, f32 SUS, f32 REL, std::string TYPE){
  attack = ATK, decay = DEC, sustain = SUS, release = REL;
  // Assume default unless type is found
  type = DEFAULT_ENV_STR;
  env_id = DEFAULT_ENV_ID;

  auto it = env_id_map.find(TYPE);
  if(it != env_id_map.end()){
    type = it->first;
    env_id = it->second;
  } 
}
