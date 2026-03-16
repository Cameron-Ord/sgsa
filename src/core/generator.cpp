#include "../../inc/synth.hpp"
#include <cmath>

f32 Generator::polyblep(f32 inc, f32 phase) const {
  if (phase < inc) {
    phase /= inc;
    return phase + phase - phase * phase - 1.0f;
  } else if (phase > 1.0f - inc) {
    phase = (phase - 1.0f) / inc;
    return phase * phase + phase + phase + 1.0f;
  }
  return 0.0;
}

f32 Generator::poly_square(f32 inc, const f32 *phase, f32 duty) const {
  if (!phase) {
    return 0.0f;
  }
  f32 sqr = square(phase, duty);
  sqr += polyblep(inc, *phase);
  sqr -= polyblep(inc, fmodf(*phase + duty, 1.0f));
  return sqr;
}

f32 Generator::poly_saw(f32 inc, const f32 *phase) const {
  if (!phase) {
    return 0.0f;
  }
  f32 saw = sawtooth(phase);
  saw -= polyblep(inc, *phase);
  return saw;
}

f32 Generator::sawtooth(const f32 *phase) const {
  if (!phase) {
    return 0.0f;
  }
  return 2.0f * *phase - 1.0f;
}

f32 Generator::square(const f32 *phase, f32 duty) const {
  if (!phase) {
    return 0.0f;
  }
  return (*phase < duty) ? 1.0f : -1.0f;
}
