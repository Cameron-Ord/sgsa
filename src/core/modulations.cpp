#include "../../inc/audio.hpp"
#include <cmath>
#include <iostream>

// Time to HZ forumla: f = 1.0 / T where f is the freq in HZ and T is the period in seconds
Amp_Modulator::Amp_Modulator(void) : tremolo_rate(1.0f), tremolo_depth(0.25f), lfo() {
  const f32 note_time = 60.0f / 120.0f * 1.0f;
  const f32  hz = 1.0f / note_time;
  std::cout << "Tremolo HZ: " << hz << std::endl;
  tremolo_rate = hz;
}

Freq_Modulator::Freq_Modulator(void) 
  : pitch_bend(1.0f), vibrato_rate(7.5f), vibrato_depth(0.0f), 
  vibrato_max(50.0f), lfo() {}

f32 Freq_Modulator::calculate_pitch_bend(f32 cents, f32 normalized_midi_event){
  const f32 bend = normalized_midi_event * cents;
  return powf(2.0f, bend * CENTS_TO_OCTAVE);
}


f32 Freq_Modulator::map_vibrato_depth(f32 normalized_midi_event){
  return normalized_midi_event * vibrato_max;
}

f32 Freq_Modulator::create_vibrato(f32 sine, f32 cents) const {
  return powf(2.0f, sine * cents * CENTS_TO_OCTAVE);
}

void Lfo::increment(f32 rate, i32 sample_rate) {
  phase += (rate / (f32)sample_rate);
  if (phase > 1.0f) {
    phase -= 1.0f;
  }
}

f32 Lfo::lfo_sine(void){
  return sinf(2.0f * PI * phase);
}
