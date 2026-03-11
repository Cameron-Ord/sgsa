#include "audio.hpp"
#include "util.hpp"
#include <cassert>
#include <cmath>

static f32 rand_f32_range(f32 min, f32 max) {
  float scale = (f32)rand() / (f32)RAND_MAX;
  return min + scale * (max - min);
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

Synth::Synth(void)
    : cfg(), osc_cfgs(1, Oscilator_Cfg()),
      wave_table(cfg, osc_cfgs),
      voices((size_t)cfg.voicings,
             Voice(cfg.low_pass_cutoff,
                   cfg.sample_rate,
                   osc_cfgs.size()
                   )) {}

void Synth::update_lpf(void){
  for(size_t i = 0; i < voices.size(); i++){
    voices[i].get_lpf().set_alpha(voices[i].get_lpf().derive_alpha(cfg.low_pass_cutoff, cfg.sample_rate));
  }
}

void Synth::loop_voicings_off(i32 midi_key) {
  for (size_t i = 0; i < (size_t)cfg.voicings; i++) {
    Voice *v = &voices[i];
    if (v->get_key() == midi_key) {
      for (size_t o = 0; o < get_osc_count(); o++) {
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

      for (size_t o = 0; o < get_osc_count(); o++) {
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

Voice::Voice(f32 cutoff, i32 sample_rate, size_t osc_count)
    : active_oscilators(0), freq(0.0f), midi_key(0), env_state(ENV_STATE::OFF),
      envelope(0.0f), oscs(osc_count, Oscilator()), lfo(),
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

void LPF::lerp(f32 target[SIZES::CHANNEL_MAX], size_t c) {
  low[c] = low[c] + (target[c] - low[c]) * alpha;
}

