#include "audio.hpp"

LPF::LPF(f32 cutoff, i32 sample_rate)
    : alpha(0.0f), low(CHANNEL_MAX) {
  reset();
  alpha = derive_alpha(cutoff, sample_rate);
}

void LPF::reset(void){
  std::fill(low.begin(), low.end(), 0.0f);
}

f32 LPF::derive_alpha(f32 cutoff, i32 sample_rate) {
  const f32 dt = 1.0f / (f32)sample_rate;
  const f32 rc = 1.0f / (2.0f * PI * cutoff);
  return dt / (rc + dt);
}

void LPF::lerp(const std::vector<f32>& target, size_t c) {
  low[c] = low[c] + (target[c] - low[c]) * alpha;
}
