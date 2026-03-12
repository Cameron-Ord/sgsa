#include "../../inc/audio.hpp"

Voice::Voice(f32 cutoff, i32 sample_rate, size_t osc_count)
    : active_oscillators(0), midi_key(0), env_state(ENV_STATE::OFF), freq(0.0f),
      envelope(0.0f), oscs(osc_count, Oscillator()), lfos(),
      voice_sums(), lpf(cutoff, sample_rate) {}

void Voice::zero_voice_sums(void){
  for(size_t i = 0; i < voice_sums.size(); i++){
    voice_sums[i] = 0.0f;
  }
}

Lfo* Voice::get_lfo_at(size_t pos) {
  if(pos < lfos.size()){
    return &lfos[pos];
  }
  return nullptr;
}


f32 Voice::get_sum_at(size_t pos) const {
  if(pos < voice_sums.size()){
    return voice_sums[pos];
  }
  return 0.0f;
}

void Voice::add_sum_at(size_t pos, f32 sample){
  if(pos < voice_sums.size()){
    voice_sums[pos] = voice_sums[pos] + sample;
  }
}

Oscillator *Voice::get_osc_at(size_t pos){
  if(pos < oscs.size()){
    return &oscs[pos];
  }
  return nullptr;
}

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
