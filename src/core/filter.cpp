#include "../../inc/audio.hpp"

LPF::LPF(f32 cutoff, i32 sample_rate)
    : alpha(0.0f), low() {
  reset();
  alpha = derive_alpha(cutoff, sample_rate);
}

void LPF::reset(void){
  std::fill(low.begin(), low.end(), 0.0f);
}

f32 LPF::get_value_at(size_t pos) const {
  if(pos < low.size()){
    return low[pos];
  }
  return 0.0f;
}

f32 LPF::derive_alpha(f32 cutoff, i32 sample_rate) {
  const f32 dt = 1.0f / (f32)sample_rate;
  const f32 rc = 1.0f / (2.0f * PI * cutoff);
  return dt / (rc + dt);
}

void LPF::lerp(const std::array<f32, CHANNEL_MAX>& target, size_t c) {
  low[c] = low[c] + (target[c] - low[c]) * alpha;
}
