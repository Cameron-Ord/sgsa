#include "../../inc/audio.hpp"

#include <cmath>

f32 Modulations::calculate_pitch_bend(f32 cents, f32 normalized_midi_event){
  const f32 bend = normalized_midi_event * cents;
  return powf(2.0f, bend * CENTS_TO_OCTAVE);
}

