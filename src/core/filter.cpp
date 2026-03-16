#include "../../inc/synth.hpp"

LPF::LPF(void) { reset(); }

void LPF::reset(void) { std::fill(low.begin(), low.end(), 0.0f); }

f32 *LPF::get_value_at(size_t pos) {
  if (pos < low.size()) {
    return &low[pos];
  }
  return nullptr;
}

const f32 *LPF::get_value_at(size_t pos) const {
  if (pos < low.size()) {
    return &low[pos];
  }
  return nullptr;
}

f32 LPF::alpha(f32 cutoff, i32 sample_rate) {
  const f32 dt = 1.0f / (f32)sample_rate;
  const f32 rc = 1.0f / (2.0f * PI * cutoff);
  return dt / (rc + dt);
}
