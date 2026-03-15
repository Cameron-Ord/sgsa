#include "../../inc/Synth.hpp"

Delay::Delay(i32 sample_rate, f32 delay_time_s, f32 _feedback) 
  : buffer((size_t)((f32)sample_rate * delay_time_s), 0.0f), 
  read(0), write(0), start(0), end(buffer.size()), feedback(_feedback) {}

void Delay::delay_write(f32 sample){
    buffer[write] = sample;
    write = (write + 1) % end;
}

f32 Delay::delay_read(void){
  f32 out = buffer[read];
  read = (read + 1) % end;
  return out;
}

void Delay::rebuild(i32 sample_rate, f32 delay_time_s){
  const f32 sample_rate_f32 = static_cast<f32>(sample_rate);
  std::vector<f32> tmp(static_cast<size_t>(sample_rate_f32 * delay_time_s));
  read = 0;
  write = 0;
  buffer = tmp;
  end = tmp.size();
}
