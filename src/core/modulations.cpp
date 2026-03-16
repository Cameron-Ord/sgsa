#include "../../inc/synth.hpp"
#include <cmath>

// Time to HZ forumla: f = 1.0 / T where f is the freq in HZ and T is the period
// in seconds

void Lfo::increment(f32 rate, i32 sample_rate) {
  phase += (rate / (f32)sample_rate);
  if (phase > 1.0f) {
    phase -= 1.0f;
  }
}

f32 Lfo::lfo_sine(void) { return sinf(2.0f * PI * phase); }
