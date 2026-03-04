#include "audio.hpp"
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
