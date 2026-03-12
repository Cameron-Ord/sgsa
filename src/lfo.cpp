#include "audio.hpp"
#include <cmath>

void Lfo::increment_lfo(f32 inc) {
  phase += inc;
  if (phase > 1.0f) {
    phase -= 1.0f;
  }
}
