#include "audio.hpp"
#include <cmath>
#include <iostream>

Waveform_Vec4f::Waveform_Vec4f(size_t _waveforms, size_t _oscillators, size_t _octaves, size_t _samples) 
  : waveforms(_waveforms), oscillators(_oscillators), octaves(_octaves), samples(_samples) {
  data.resize(waveforms * oscillators * octaves * samples);
  std::cout << "Created wavetable buffer with " << waveforms * oscillators * octaves * samples << " elements" << std::endl;
  std::fill(data.begin(), data.end(), 0.0f);
}

size_t Waveform_Vec4f::index(size_t wave_p, size_t osc_p, size_t oct_p, size_t sample_p) const {
  return wave_p * (oscillators * octaves * samples) + osc_p * (octaves * samples) + oct_p * samples + sample_p;
}

bool Waveform_Vec4f::valid(size_t pos) const {
  if(pos >= (waveforms * oscillators * octaves * samples)){
    return false;
  }
  return true;
}

const f32 *Waveform_Vec4f::get_at(size_t wave_p, size_t osc_p, size_t oct_p, size_t sample_p) const {
  const size_t i = index(wave_p, osc_p, oct_p, sample_p);
  if(valid(i)){
    return &data[i];
  }
  return nullptr;
}

bool Waveform_Vec4f::set_at(size_t wave_p, size_t osc_p, size_t oct_p, size_t sample_p, f32 val){
  const size_t i = index(wave_p, osc_p, oct_p, sample_p);
  if(valid(i)){
    data[i] = val;
    return true;
  }
  return false;
}

size_t Wave_Table::index_octave(f32 freq) const {
  for (u8 i = 0; i < SIZES::OCTAVES - 1; i++) {
    const f32 base = freq_range[i];
    const f32 next = freq_range[i + 1];
    if (freq >= base && freq < next) {
      return i;
    }
  }
  return SIZES::OCTAVES - 1;
}

void Wave_Table::sine(Waveform_Vec4f& v, size_t osc, size_t octave, size_t N){
  for (size_t n = 0; n < N; n++) {
    const f32 phase = (f32)n / (f32)N;
    v.set_at(SINE, osc, octave, n, sinf(2.0f * PI * phase));
  }
}

void Wave_Table::fourier_saw(Waveform_Vec4f& v, size_t osc, size_t octave, size_t N, size_t harm){
  for (size_t n = 0; n < N; n++) {
    const f32 phase = (f32)n / (f32)N;
    f32 sum = 0.0f;
    for (size_t k = 1; k <= harm; k++) {
      f32 sign = powf(-1.0f, (f32)k);
      sum += sign * sinf(2.0f * PI * (f32)k * phase) / (f32)k;
    }
    v.set_at(SAW, osc, octave, n, -(2.0f / PI) * sum);
  }
}
void Wave_Table::fourier_square(Waveform_Vec4f& v, size_t osc, size_t octave, size_t N, size_t harm){
  for (size_t n = 0; n < N; n++) {
    const f32 phase = (f32)n / (f32)N;
    f32 sum = 0.0f;
    for (size_t k = 1; k <= harm; k++) {
      sum += 1.0f * sinf(2.0f * PI * (2.0f * (f32)k - 1.0f) * phase) /
             (2.0f * (f32)k - 1.0f);
    }
    v.set_at(SQUARE, osc, octave, n, (4.0f / PI) * sum);
  }
}
void Wave_Table::fourier_triangle(Waveform_Vec4f& v, size_t osc, size_t octave, size_t N, size_t harm){
  for (size_t n = 0; n < N; n++) {
    const f32 phase = (f32)n / (f32)N;
    f32 sum = 0.0f;
    for (size_t k = 1; k <= harm; k++) {
      f32 sign = powf(-1.0f, (f32)k);
      sum += sign * sinf(2.0f * PI * (2.0f * (f32)k - 1.0f) * phase) /
             powf(2.0f * (f32)k - 1.0f, 2.0f);
    }
    v.set_at(TRIANGLE, osc, octave, n, -(8.0f / powf(PI, 2.0f)) * sum);
  }
}

void Wave_Table::fourier_pulse(Waveform_Vec4f& v, size_t osc, size_t octave, size_t N, size_t harm, f32 duty_cycle){
  for (size_t n = 0; n < N; n++) {
    const f32 phase = (f32)n / (f32)N;
    f32 sum = 0.0f;
    for (size_t k = 1; k <= harm; k++) {
      sum += (1.0f / (f32)k) * sinf(PI * (f32)k * duty_cycle) *
             cosf(2.0f * PI * (f32)k * phase);
    }
    v.set_at(PULSE, osc, octave, n, (duty_cycle + (2.0f / PI) * sum) - 1.0f);
  }
}

// Need to break this up into delegated functions and add a square wave
void Wave_Table::generate(Synth_Cfg scfg, std::vector<Oscilator_Cfg> ocfgs) {
  if (scfg.wave_table_size > MAX_TABLE_SIZE) {
    scfg.wave_table_size = MAX_TABLE_SIZE;
  }
  const size_t N = scfg.wave_table_size;
  const f32 C0 = 8.1758f;

  for(size_t octave = 0; octave < OCTAVES; octave++){
      const f32 base_freq = C0 * powf(2.0f, (f32)octave);
      freq_range[octave] = base_freq;
  }

  for(size_t osc = 0; osc < ocfgs.size(); osc++){
    for (size_t octave = 0; octave < OCTAVES; octave++) {

      const f32 nyquist = (f32)scfg.sample_rate / 2.0f;
      const size_t harm = (size_t)floorf(nyquist / freq_range[octave]);

      fourier_saw(table, osc, octave, N, harm);
      fourier_square(table, osc, octave, N, harm);
      fourier_triangle(table, osc, octave, N, harm);
      fourier_pulse(table, osc, octave, N, harm, ocfgs[osc].duty);
      sine(table, osc, octave, N);
    }
  }
}

Wave_Table::Wave_Table(Synth_Cfg scfg, std::vector<Oscilator_Cfg> ocfgs)
    : table(WAVEFORM_COUNT, MAX_OSC_COUNT, OCTAVES, MAX_TABLE_SIZE), freq_range(OCTAVES) {
  generate(scfg, ocfgs);
}
