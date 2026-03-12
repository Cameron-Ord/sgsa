#include "../../inc/audio.hpp"
#include "../../inc/util.hpp"

Synth::Synth(void)
    : synth_cfg(), env_cfg(), osc_cfgs(1, Oscillator_Cfg()), lfo_cfgs(),
      voices(synth_cfg.voicings, Voice(synth_cfg.low_pass_cutoff, synth_cfg.sample_rate, osc_cfgs.size())), 
      wave_table(synth_cfg, osc_cfgs), delay(synth_cfg.sample_rate, 0.5f, 0.5f), mods(), loop_sums() {}


void Synth::zero_loop_sums(void){
   for(size_t i = 0; i < loop_sums.size(); i++){
      loop_sums[i] = 0.0f;
   }
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

const Oscillator_Cfg *Synth::get_osc_cfg_at(size_t pos) const {
  if(pos < osc_cfgs.size()){
    return &osc_cfgs[pos];
  }
  return nullptr;
}

void Synth::update_lpf(void){
  for(size_t i = 0; i < voices.size(); i++){
    voices[i].get_lpf().set_alpha(voices[i].get_lpf().derive_alpha(synth_cfg.low_pass_cutoff, synth_cfg.sample_rate));
  }
}

const Lfo_Cfg *Synth::get_lfo_cfg_at(size_t pos){
  if(pos < lfo_cfgs.size()){
    return &lfo_cfgs[pos];
  }
  return nullptr;
}

void Synth::loop_voicings_off(i32 midi_key) {
  for (size_t i = 0; i < synth_cfg.voicings; i++) {
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
  for (size_t i = 0; i < synth_cfg.voicings; i++) {
    Voice *v = &voices[i];
    if (v->get_active_count() <= 0 && v->done()) {
      v->set_active_count(0);
      v->set_key(midi_key);
      v->set_freq(midi_to_freq(midi_key));
      v->get_lpf().reset();

      for (size_t o = 0; o < get_osc_count(); o++) {
        Oscillator *osc = &v->get_osc_array()[o];
        osc->start();
        v->set_active_count(v->get_active_count() + 1);
      }
      v->set_envelope(0.0f);
      v->set_env_state(ENV_STATE::ATK);
      return;
    }
  }
}
