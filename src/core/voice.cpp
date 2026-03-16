#include "../../inc/synth.hpp"
#include <cmath>
#include <iostream>

Voice::Voice(void)
    : active_oscillators(0), midi_key(0), env_state(ENV_STATE::OFF), freq(0.0f),
      envelope(0.0f), volume_multiplier(1.0f), lpf(), vibrato(), tremolo(),
      voice_sums(), clipped_sums(), filtered_sums(), voice_out() {}

void Voice::set_filtered_at(size_t pos, const f32 *val) {
  if (!val) {
    return;
  }
  if (pos < filtered_sums.size()) {
    filtered_sums[pos] = *val;
  }
}

void Voice::set_clipped_at(size_t pos, const f32 *val) {
  if (!val) {
    return;
  }
  if (pos < clipped_sums.size()) {
    clipped_sums[pos] = *val;
  }
}

void Voice::set_out_at(size_t pos, const f32 *val) {
  if (!val) {
    return;
  }
  if (pos < voice_out.size()) {
    voice_out[pos] = *val;
  }
}

void Voice::set_filtered_at(size_t pos, f32 val) {
  if (pos < filtered_sums.size()) {
    filtered_sums[pos] = val;
  }
}

void Voice::set_clipped_at(size_t pos, f32 val) {
  if (pos < clipped_sums.size()) {
    clipped_sums[pos] = val;
  }
}

void Voice::set_out_at(size_t pos, f32 val) {
  if (pos < voice_out.size()) {
    voice_out[pos] = val;
  }
}

void Voice::zero_voice_sums(void) {
  for (size_t i = 0; i < voice_sums.size(); i++) {
    voice_sums[i] = 0.0f;
  }
}

void Voice::add_sum_at(size_t pos, const f32 *sample) {
  if (!sample) {
    return;
  }
  if (pos < voice_sums.size()) {
    voice_sums[pos] = voice_sums[pos] + *sample;
  }
}

const f32 *Voice::get_filtered_at(size_t pos) const {
  if (pos < filtered_sums.size()) {
    return &filtered_sums[pos];
  }
  return nullptr;
}

const f32 *Voice::get_sum_at(size_t pos) const {
  if (pos < voice_sums.size()) {
    return &voice_sums[pos];
  }
  return nullptr;
}

const f32 *Voice::get_clipped_at(size_t pos) const {
  if (pos < clipped_sums.size()) {
    return &clipped_sums[pos];
  }
  return nullptr;
}

const f32 *Voice::get_out_at(size_t pos) const {
  if (pos < voice_out.size()) {
    return &voice_out[pos];
  }
  return nullptr;
}

// https://en.wikipedia.org/wiki/Exponential_smoothing
f32 Voice::get_env_alpha(f32 dt, f32 time) {
  return 1.0f - expf(-dt / -(time / logf(1.0f - 0.96f)));
}

bool Voice::done(void) const { return env_state == ENV_STATE::OFF; }

bool Voice::releasing(void) const { return env_state == ENV_STATE::REL; }

void Voice::adsr(f32 dt, f32 atk, f32 dec, f32 sus, f32 rel) {
  const f32 EPS = 1.0f - 0.95f, ZERO = 0.0f, ONE = 1.0f;
  switch (env_state) {
  default:
    return;
  case ENV_STATE::ATK: {
    lerp_f32(&ONE, &envelope, get_env_alpha(dt, atk));
    if (envelope >= 1.0f - EPS) {
      envelope = 1.0f;
      env_state = ENV_STATE::DEC;
    }
  } break;

  case ENV_STATE::DEC: {
    lerp_f32(&sus, &envelope, get_env_alpha(dt, dec));
    if (envelope <= sus + EPS) {
      envelope = sus;
      env_state = ENV_STATE::SUS;
    }
  } break;

  case ENV_STATE::REL: {
    lerp_f32(&ZERO, &envelope, get_env_alpha(dt, rel));
    if (envelope <= 0.0f + EPS) {
      envelope = 0.0f;
      env_state = ENV_STATE::OFF;
    }
  } break;
  }
}
