#include "audio.hpp"
#include "util.hpp"

Oscilator::Oscilator(void) : gen(CHANNEL_MAX), phase(0.0f), time(0.0f) {}

void Oscilator::start(void) {
  phase = rand_f32_range(0.0f, 0.1f);
  time = 0.0f;
}

void Oscilator::increment_time(f32 dt) { time += dt; }

void Oscilator::increment_phase(f32 inc, f32 max) {
  phase += inc;
  if (phase >= max) {
    phase -= max;
  }
}
