#include "../../inc/synth.hpp"
#include "../../inc/util.hpp"

Oscillator::Oscillator(void) : gen(), phase(), time() {}

f32 Oscillator::phase_clamp(f32 phase_val, f32 max) {
  if(phase_val < 0.0f){
    phase_val += max;
  } else if(phase_val > max){
    phase_val -= max;
  }
  return phase_val;
}

std::array<f32, CHANNEL_MAX>* Oscillator::get_sample_array_at(size_t pos){
  if(pos < VOICES){
    return &gen[pos];
  }
  return nullptr;
}

f32 Oscillator::get_phase_at(size_t pos) const {
  if(pos < VOICES){
    return phase[pos];
  }
  return 0.0f;
}

f32 Oscillator::get_time_at(size_t pos) const {
  if(pos < VOICES){
    return time[pos];
  }
  return 0.0f;
}

f32 Oscillator::get_sample_at(const std::array<f32, CHANNEL_MAX>* buf, size_t pos) const {
  if(buf && pos < buf->size()){
    return (*buf)[pos];
  }
  return 0.0f;
}

void Oscillator::set_sample_at(std::array<f32, CHANNEL_MAX>* buf, size_t pos, f32 value) {
  if(buf && pos < buf->size()){
    (*buf)[pos] = value;
  }
}

void Oscillator::reset(size_t voice_index) {
  if(voice_index < VOICES){
    phase[voice_index] = rand_f32_range(0.0f, 0.5f);
    time[voice_index] = 0.0f;
  }
}

void Oscillator::increment_time_at(f32 dt, size_t pos) { 
  if(pos < VOICES){
    time[pos] += dt;
  }
}

void Oscillator::increment_phase_at(f32 inc, f32 max, size_t pos) {
  if(pos < VOICES){
    phase[pos] += inc;
    if (phase[pos] >= max) {
      phase[pos] -= max;
    }
  }
}
