#include "../../inc/audio.hpp"
#include "../../inc/util.hpp"
#include "../../inc/controller.hpp"

#include <cmath>

const size_t DEFAULT_OSC_COUNT = 1;
const f32 DEFAULT_DELAY_TIME = 0.5f;
const f32 DEFAULT_DELAY_FEEDBACK = 0.5f;

Synth::Synth(void)
    : oscs(DEFAULT_OSC_COUNT), voices(), generator(), 
    delay(sample_rate, DEFAULT_DELAY_TIME, DEFAULT_DELAY_FEEDBACK), loop_sums() {}

void Synth::update_fmod(f32 normalized_event, i32 type){
  switch(type){
    default: break;
    case CONTROL_MOD_WHEEL: {
      set_vibrato_depth(map_vibrato_depth(normalized_event));
    } break;
    case PITCH_BEND: {
      set_pitch_bend(calculate_pitch_bend(TWO_SEMITONE_CENTS, normalized_event));
    } break;
  }
}

f32 Synth::calculate_pitch_bend(f32 cents, f32 normalized_midi_event) const {
  const f32 bend = normalized_midi_event * cents;
  return powf(2.0f, bend * CENTS_TO_OCTAVE);
}

f32 Synth::map_vibrato_depth(f32 normalized_midi_event) const {
  return normalized_midi_event * vibrato_max;
}

void Synth::zero_loop_sums(void){
   for(size_t i = 0; i < loop_sums.size(); i++){
      loop_sums[i] = 0.0f;
   }
}

Oscillator *Synth::get_osc_at(size_t pos){
  if(pos < oscs.size()){
    return &oscs[pos];
  }
  return nullptr;
}

f32 Synth::get_sum_at(size_t pos){
  if(pos < loop_sums.size()){
    return loop_sums[pos];
  }
  return 0.0f;
}

void Synth::add_sum_at(size_t pos, f32 sum){
  if(pos < loop_sums.size()){
    loop_sums[pos] = loop_sums[pos] + sum;
  }
}

void Synth::loop_voicings_off(i32 midi_key) {
  for (size_t i = 0; i < voices.size(); i++) {
    Voice *v = &voices[i];
    if (v->get_key() == midi_key) {
      for (size_t o = 0; o < oscs.size(); o++) {
        v->set_active_count(v->get_active_count() - 1);
      }
      v->set_env_state(ENV_STATE::REL);
    }
  }
}

void Synth::loop_voicings_on(i32 midi_key, f32 normalized_velocity) {
  for (size_t i = 0; i < voices.size(); i++) {
    Voice *v = &voices[i];
    if (v->get_active_count() <= 0 && v->done()) {
      v->set_active_count(0);
      v->set_key(midi_key);
      v->set_freq(midi_to_freq(midi_key));
      v->get_lpf().reset();

      for (size_t o = 0; o < oscs.size(); o++) {
        oscs[o].reset(i);
        v->set_active_count(v->get_active_count() + 1);
      }

      v->set_envelope(0.0f);
      v->set_env_state(ENV_STATE::ATK);
      v->set_vol_mult(1.0f + normalized_velocity);

      for(size_t c = 0; c < static_cast<size_t>(channels); c++){
        v->set_clipped_at(c, 0.0f);
        v->set_filtered_at(c, 0.0f);
        v->set_out_at(c, 0.0f);
      }
      return;
    }
  }
}
