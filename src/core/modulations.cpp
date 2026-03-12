#include "../../inc/audio.hpp"
#include <cmath>

Modulations::Modulations(void) 
  : pitch_bend(1.0f), vibrato_depth(0.0f), vibrato_max(30.0f) {}

f32 Modulations::calculate_pitch_bend(f32 cents, f32 normalized_midi_event){
  const f32 bend = normalized_midi_event * cents;
  return powf(2.0f, bend * CENTS_TO_OCTAVE);
}


f32 Modulations::map_vibrato_depth(f32 normalized_midi_event){
  return normalized_midi_event * vibrato_max;
}

const Lfo_Cfg *Modulations::lfo_cfg_at(size_t pos) const {
  if(pos < lfo_cfgs.size()){
    return &lfo_cfgs[pos];
  }
  return nullptr;
}

void Lfo::increment(f32 rate, i32 sample_rate) {
  phase += (rate / (f32)sample_rate);
  if (phase > 1.0f) {
    phase -= 1.0f;
  }
}

f32 Lfo::lfo_sine(f32 depth_cents){
  return powf(2.0f, sinf(2.0f * PI * phase) * depth_cents * CENTS_TO_OCTAVE);
}
