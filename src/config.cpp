#include "audio.hpp"
#include <toml++/toml.hpp>
#include <iostream>
#include <unordered_map>


const std::unordered_map<std::string, WAVEFORM_TYPE> table_id_map = {
  { "SAW",  WAVEFORM_TYPE::SAW },
  { "SINE", WAVEFORM_TYPE::SINE },
};

const std::unordered_map<std::string, ENV_TYPE> env_id_map = {
  { "ADSR", ENV_TYPE::ADSR },
  { "AR", ENV_TYPE::AR }
};

const std::unordered_map<std::string, LFO_TYPE> lfo_id_map = {
  { "TREMOLO", LFO_TYPE::TREMOLO },
  { "VIBRATO", LFO_TYPE::VIBRATO }
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

Params::Params(Lfo_Params lfop_tmp, Env_Params envp_tmp, Audio_Params ap_tmp)
  : lfop(lfop_tmp), envp(envp_tmp), ap(ap_tmp) {}

Oscilator_Cfg::Oscilator_Cfg(f32 CYCLE, std::string NAME, f32 DETUNE, f32 VOLUME, f32 STEP){
  cycle = CYCLE;
  detune = DETUNE, volume = VOLUME, step = STEP;
  auto it = table_id_map.find(NAME);
  if(it != table_id_map.end()){
    table_id = it->second;
  }
}

Lfo_Params::Lfo_Params(f32 RATE, f32 DEPTH, f32 TIMER, std::string MODESTR, bool active){
  rate = RATE, depth = DEPTH, timer = TIMER;
  on = active;
  auto it = lfo_id_map.find(MODESTR);
  if(it != lfo_id_map.end()){
    mode = it->second;
  }
}

Audio_Params::Audio_Params(void){
  const f32 LOW_CUTOFF_FREQ = 80.0f;
  const f32 DT = 1.0f / (f32)sample_rate;
  const f32 RC_HIGH = 1.0f / (2.0f * PI * NYQUIST((f32)sample_rate));
  const f32 RC_LOW = 1.0f / (2.0f * PI * LOW_CUTOFF_FREQ);

  const f32 ALPHA_HIGH = DT / (RC_HIGH + DT);
  const f32 ALPHA_LOW = DT / (RC_LOW + DT);

  lpf_alpha_high = ALPHA_HIGH;
  lpf_alpha_low = ALPHA_LOW;
}

Audio_Params::Audio_Params(i32 CHANNELS, i32 SAMPLE_RATE, size_t VOICE_COUNT, 
    size_t WT_SIZE, f32 TEMPO, f32 NOTE_DUR, f32 CUTOFF_LOW, f32 CUTOFF_HIGH, f32 VOLUME, f32 GAIN){
  channels = CHANNELS, sample_rate = SAMPLE_RATE;
  voicings = VOICE_COUNT, wave_table_size = WT_SIZE;
  tempo = TEMPO, note_duration = NOTE_DUR;
  volume = VOLUME, gain = GAIN;

  const f32 DT = 1.0f / (f32)SAMPLE_RATE;
  const f32 RC_HIGH = 1.0f / (2.0f * PI * CUTOFF_HIGH);
  const f32 RC_LOW = 1.0f / (2.0f * PI * CUTOFF_LOW);

  const f32 ALPHA_HIGH = DT / (RC_HIGH + DT);
  const f32 ALPHA_LOW = DT / (RC_LOW + DT);

  lpf_alpha_high = ALPHA_HIGH;
  lpf_alpha_low = ALPHA_LOW;
}

Env_Params::Env_Params(f32 ATK, f32 DEC, f32 SUS, f32 REL, std::string TYPE){
  attack = ATK, decay = DEC, sustain = SUS, release = REL;
  auto it = env_id_map.find(TYPE);
  if(it != env_id_map.end()){
    env_id = it->second;
  } 
}
