#include "../../inc/audio.hpp"
#include "../../inc/util.hpp"

Oscillator::Oscillator(void) : gen(), phase(0.0f), time(0.0f) {}

f32 Oscillator::phase_clamp(f32 phase_val, f32 max) {
  if(phase_val < 0.0f){
    phase_val += max;
  } else if(phase_val > max){
    phase_val -= max;
  }
  return phase_val;
}

f32 Oscillator::get_sample_at(size_t pos) const {
  if(pos >= gen.size()){
    return 0.0f;
  }
  return gen[pos];
}

void Oscillator::mult_sample_at(size_t pos, f32 factor){
  if(pos < gen.size()){
    gen[pos] = gen[pos] * factor;
  }
}

void Oscillator::set_sample_at(size_t pos, f32 value) {
  if(pos < gen.size()){
    gen[pos] = value;
  }
}

void Oscillator::start(void) {
  phase = rand_f32_range(0.0f, 0.1f);
  time = 0.0f;
}

void Oscillator::increment_time(f32 dt) { time += dt; }

void Oscillator::increment_phase(f32 inc, f32 max) {
  phase += inc;
  if (phase >= max) {
    phase -= max;
  }
}
