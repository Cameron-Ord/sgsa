#include "audio.hpp"
#include "util.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

static f32 rand_f32_range(f32 min, f32 max) {
  float scale = (f32)rand() / (f32)RAND_MAX;
  return min + scale * (max - min);
}

size_t Wave_Table::index_octave(f32 freq) const {
  for (u8 i = 0; i < SIZES::OCTAVES - 1; i++) {
    const f32 base = freq_mapper[i];
    const f32 next = freq_mapper[i + 1];
    if (freq >= base && freq < next) {
      return i;
    }
  }
  return SIZES::OCTAVES - 1;
}

const f32 *Wave_Table::get_table(size_t id, size_t index) const {
  if (index >= SIZES::MAX_TABLE_SIZE) {
    return NULL;
  }

  if (id >= SIZES::OCTAVES) {
    return NULL;
  }

  return tables[id][index];
}

void Wave_Table::re_generate(i32 sample_rate, size_t table_size,
                             f32 duty_cycle) {
  size = table_size;
  generate(sample_rate, duty_cycle);
}

void Wave_Table::fourier_saw(f32 buf[SIZES::MAX_TABLE_SIZE], size_t N,
                             size_t harm) {
  for (size_t n = 0; n < N; n++) {
    const f32 phase = (f32)n / (f32)N;
    f32 sum = 0.0f;
    for (size_t k = 1; k <= harm; k++) {
      f32 sign = powf(-1.0f, (f32)k);
      sum += sign * sinf(2.0f * PI * (f32)k * phase) / (f32)k;
    }
    buf[n] = -(2.0f / PI) * sum;
  }
}

void Wave_Table::fourier_square(f32 buf[SIZES::MAX_TABLE_SIZE], size_t N,
                                size_t harm) {
  for (size_t n = 0; n < N; n++) {
    const f32 phase = (f32)n / (f32)N;
    f32 sum = 0.0f;
    for (size_t k = 1; k <= harm; k++) {
      sum += 1.0f * sinf(2.0f * PI * (2.0f * (f32)k - 1.0f) * phase) /
             (2.0f * (f32)k - 1.0f);
    }
    buf[n] = (4.0f / PI) * sum;
  }
}

void Wave_Table::fourier_triangle(f32 buf[SIZES::MAX_TABLE_SIZE], size_t N,
                                  size_t harm) {
  for (size_t n = 0; n < N; n++) {
    const f32 phase = (f32)n / (f32)N;
    f32 sum = 0.0f;
    for (size_t k = 1; k <= harm; k++) {
      f32 sign = powf(-1.0f, (f32)k);
      sum += sign * sinf(2.0f * PI * (2.0f * (f32)k - 1.0f) * phase) /
             powf(2.0f * (f32)k - 1.0f, 2.0f);
    }
    buf[n] = -(8.0f / powf(PI, 2.0f)) * sum;
  }
}

void Wave_Table::fourier_pulse(f32 buf[SIZES::MAX_TABLE_SIZE], size_t N,
                               size_t harm, f32 duty_cycle) {
  for (size_t n = 0; n < N; n++) {
    const f32 phase = (f32)n / (f32)N;
    f32 sum = 0.0f;
    for (size_t k = 1; k <= harm; k++) {
      sum += (1.0f / (f32)k) * sinf(PI * (f32)k * duty_cycle) *
             cosf(2.0f * PI * (f32)k * phase);
    }
    buf[n] = (duty_cycle + (2.0f / PI) * sum) - 1.0f;
  }
}

// stupid stinky regular sine with no harmonics that gets brutally mogged by
// sawtooth chad.
void Wave_Table::sine(f32 buf[SIZES::MAX_TABLE_SIZE], size_t N) {
  for (size_t n = 0; n < N; n++) {
    const f32 phase = (f32)n / (f32)N;
    buf[n] = sinf(2.0f * PI * phase);
  }
}

// Need to break this up into delegated functions and add a square wave
void Wave_Table::generate(i32 sample_rate, f32 duty_cycle) {
  const size_t WAVE_SIZE_MAX = 1 << 12;
  if (size > WAVE_SIZE_MAX) {
    size = WAVE_SIZE_MAX;
  }
  const size_t N = size;
  // C0 lowest note on the piano realistically possible
  const f32 C0 = 16.35f;
  const f32 C1 = 32.703f;

  for (size_t o = 0; o < OCTAVES; o++) {
    // Calculate the base of the current octave
    // Find the end of the current harmonic content of this octave
    // by dividing the nyquist by the top of the current octave
    const f32 base_freq = C0 * powf(2.0f, (f32)o);
    const f32 last_freq = C1 * powf(2.0f, (f32)o);

    const f32 nyquist = (f32)sample_rate / 2.0f;
    const size_t harm = (size_t)floorf(nyquist / last_freq);
    freq_mapper[o] = base_freq;

    fourier_saw(tables[WAVEFORM_TYPE::SAW][o], N, harm);
    fourier_square(tables[WAVEFORM_TYPE::SQUARE][o], N, harm);
    fourier_triangle(tables[WAVEFORM_TYPE::TRIANGLE][o], N, harm);
    fourier_pulse(tables[WAVEFORM_TYPE::PULSE][o], N, harm, duty_cycle);
    sine(tables[WAVEFORM_TYPE::SINE][o], N);
  }
}

Wave_Table::Wave_Table(i32 sample_rate, size_t table_size, f32 duty_cycle)
    : tables(), freq_mapper(), size(table_size) {
  generate(sample_rate, duty_cycle);
}

Synth::Synth(void)
    : cfg(),
      wave_table(cfg.sample_rate, (size_t)cfg.wave_table_size, cfg.duty_cycle),
      voices((size_t)cfg.voicings,
             Voice(cfg.low_pass_cutoff,
                   cfg.sample_rate)) {}

void Synth::new_oscilators(std::vector<Oscilator_Cfg> osc_cfgs) {
  for (size_t i = 0; i < voices.size(); i++) {
    std::vector<Oscilator> &oscs = voices[i].get_osc_array();
    oscs.resize(osc_cfgs.size());
    for (size_t j = 0; j < oscs.size(); j++) {
      oscs[j] = Oscilator(osc_cfgs[j]);
    }
  }
}

void Synth::loop_voicings_off(i32 midi_key) {
  for (size_t i = 0; i < (size_t)cfg.voicings; i++) {
    Voice *v = &voices[i];
    if (v->get_key() == midi_key) {
      for (size_t o = 0; o < v->get_osc_count(); o++) {
        v->set_active_count(v->get_active_count() - 1);
      }
      v->set_env_state(ENV_STATE::REL);
    }
  }
}

void Synth::loop_voicings_on(i32 midi_key) {
  for (size_t i = 0; i < (size_t)cfg.voicings; i++) {
    Voice *v = &voices[i];
    if (v->get_active_count() <= 0 && v->done()) {
      v->set_active_count(0);
      v->set_key(midi_key);
      v->set_freq(midi_to_freq(midi_key));
      v->get_lpf().reset();

      for (size_t o = 0; o < v->get_osc_count(); o++) {
        Oscilator *osc = &v->get_osc_array()[o];
        osc->start();
        v->set_active_count(v->get_active_count() + 1);
      }
      v->set_envelope(0.0f);
      v->set_env_state(ENV_STATE::ATK);
      return;
    }
  }
}

f32 Lfo::vibrato(f32 depth) { return depth * sinf(2.0f * PI * phase); }

void Lfo::increment_lfo(f32 inc) {
  phase += inc;
  if (phase > 1.0f) {
    phase -= 1.0f;
  }
}

Oscilator::Oscilator(void) : gen(), cfg(), phase(0.0f), time(0.0f) {}
Oscilator::Oscilator(Oscilator_Cfg _cfg) : gen(), cfg(_cfg), phase(0.0f), time(0.0f) {}

void Oscilator::start(void) {
  phase = 0.0f;
  time = 0.0f;
}

void Oscilator::increment_time(f32 dt) { time += dt; }

void Oscilator::increment_phase(f32 inc, f32 max) {
  phase += inc;
  if (phase >= max) {
    phase -= max;
  }
}

Voice::Voice(f32 cutoff, i32 sample_rate)
    : active_oscilators(0), freq(0.0f), midi_key(0), env_state(ENV_STATE::OFF),
      envelope(0.0f), oscs(1, Oscilator()), lfo(),
      lpf(cutoff, sample_rate) {}

bool Voice::done(void) const { return env_state == ENV_STATE::OFF; }

bool Voice::releasing(void) const { return env_state == ENV_STATE::REL; }

void Voice::adsr(i32 samplerate, f32 atk, f32 dec, f32 sus, f32 rel) {
  switch (env_state) {
  default:
    return;
  case ENV_STATE::ATK: {
    envelope += ATTACK_INCREMENT((f32)samplerate, atk);
    if (envelope >= 1.0f) {
      envelope = 1.0f;
      env_state = ENV_STATE::DEC;
    }
  } break;

  case ENV_STATE::DEC: {
    envelope -= DECAY_INCREMENT((f32)samplerate, dec, sus);
    if (envelope <= sus) {
      envelope = sus;
      env_state = ENV_STATE::SUS;
    }
  } break;

  case ENV_STATE::REL: {
    envelope -= RELEASE_INCREMENT((f32)samplerate, rel);
    if (envelope <= 0.0f) {
      envelope = 0.0f;
      env_state = ENV_STATE::OFF;
    }
  } break;
  }
}

void Voice::ar(i32 samplerate, f32 atk, f32 rel) {
  switch (env_state) {
  default:
    break;
  case ENV_STATE::ATK: {
    envelope += ATTACK_INCREMENT((f32)samplerate, atk);
    if (envelope >= 1.0f) {
      envelope = 1.0f;
      env_state = ENV_STATE::REL;
    }
  } break;
  case ENV_STATE::REL: {
    envelope -= RELEASE_INCREMENT((f32)samplerate, rel);
    if (envelope <= 0.0f) {
      envelope = 0.0f;
      env_state = ENV_STATE::OFF;
    }
  } break;
  }
}

LPF::LPF(f32 cutoff, i32 sample_rate)
    : alpha(0.0f) {
  reset();
  alpha = derive_alpha(cutoff, sample_rate);
}

void LPF::reset(void){
  memset(low, 0, sizeof(f32) * SIZES::CHANNEL_MAX);
}

f32 LPF::derive_alpha(f32 cutoff, i32 sample_rate) {
  const f32 dt = 1.0f / (f32)sample_rate;
  const f32 rc = 1.0f / (2.0f * PI * cutoff);
  return dt / (rc + dt);
}

void LPF::lerp(f32 target[SIZES::CHANNEL_MAX], i32 c) {
  low[c] = low[c] + (target[c] - low[c]) * alpha;
}

