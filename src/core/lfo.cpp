#include "../../inc/audio.hpp"
#include <cmath>

void Lfo::increment(f32 rate, i32 sample_rate) {
  phase += (rate / (f32)sample_rate);
  if (phase > 1.0f) {
    phase -= 1.0f;
  }
}

f32 Lfo::lfo_sine(f32 depth){
  return 1.0f + sinf(2.0f * PI * phase) * depth;
}
