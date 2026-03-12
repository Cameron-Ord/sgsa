#include "audio.hpp"
#include "util.hpp"

Synth::Synth(void)
    : synth_cfg(), env_cfg(), osc_cfgs(1, Oscilator_Cfg()), lfo_cfgs(1, Lfo_Cfg()),
      voices(synth_cfg.voicings, Voice(synth_cfg.low_pass_cutoff, synth_cfg.sample_rate, osc_cfgs.size(), lfo_cfgs.size())), 
      wave_table(synth_cfg, osc_cfgs), loop_sums(CHANNEL_MAX, 0.0f) {}


void Synth::zero_loop_sums(void){
   for(size_t i = 0; i < loop_sums.size(); i++){
      loop_sums[i] = 0.0f;
   }
}

void Synth::update_lpf(void){
  for(size_t i = 0; i < voices.size(); i++){
    voices[i].get_lpf().set_alpha(voices[i].get_lpf().derive_alpha(synth_cfg.low_pass_cutoff, synth_cfg.sample_rate));
  }
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
