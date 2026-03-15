#include "../../inc/synth.hpp"
#include "../../inc/util.hpp"

#include <cmath>

const size_t DEFAULT_OSC_COUNT = 1;
const f32 DEFAULT_DELAY_TIME = 0.5f;
const f32 DEFAULT_DELAY_FEEDBACK = 0.5f;

Synth::Synth(void)
    : params(init_params()), oscs(DEFAULT_OSC_COUNT), voices(), generator(), 
    delay(sample_rate, DEFAULT_DELAY_TIME, DEFAULT_DELAY_FEEDBACK), loop_sums() {}


std::array<ParamF32, S_PARAM_COUNT> Synth::init_params(void){
  // MIN - MAX - VALUE - INC
  return {
    ParamF32("Attack", 0.01f, 1.5f, 0.01f, 0.05f),
    ParamF32("Decay", 0.01f, 1.5f, 0.3f, 0.05f),
    ParamF32("Sustain", 0.1f, 1.5f, 0.8f, 0.05f),
    ParamF32("Release", 0.01f, 1.5f, 0.05f, 0.05f),
    ParamF32("Volume", 0.0f, 1.0f, 1.0f, 0.1f),
    ParamF32("Gain", 1.0f, 10.0f, 1.0f, 1.0f),
    ParamF32("Low-Pass", 100.0f, 8000.0f, 2000.0f, 100.0f),
    ParamF32("Tremolo", 0.1f, 1.0, 0.25f, 0.1f),
    ParamF32("Delay Time", 0.1f, 2.0f, 0.5f, 0.25f),
  };
}


void Synth::inc_param(SYNTH_PARAMETER param){
  if(param < params.size()){
    const f32 min = params[param].min;
    const f32 max = params[param].max;
    const f32 base_val = params[param].value;
    const f32 inc_val = params[param].inc;

    params[param].value = clamp_param_f32(min, max, base_val + inc_val);
    if(param == S_DELAY_TIME){
      delay.rebuild(sample_rate, params[param].value);
    }
  }
}

void Synth::dec_param(SYNTH_PARAMETER param){
  if(param < params.size()){
    const f32 min = params[param].min;
    const f32 max = params[param].max;
    const f32 base_val = params[param].value;
    const f32 inc_val = -params[param].inc;

    params[param].value = clamp_param_f32(min, max, base_val + inc_val);
    if(param == S_DELAY_TIME){
      delay.rebuild(sample_rate, params[param].value);
    }
  }
}

f32 Synth::clamp_param_f32(f32 min, f32 max, f32 value){
  if(value < min){
    value = min;
  }

  if(value > max){
    value = max;
  }

  return value;
}

void Synth::set_param(SYNTH_PARAMETER param, f32 value){
  if(param < params.size()){
    params[param].value = clamp_param_f32(params[param].min, params[param].max, value);
  }
}

ParamF32 *Synth::get_param(SYNTH_PARAMETER param) {
  if(param < params.size()){
    return &params[param];
  }

  return nullptr;
}

const ParamF32 *Synth::get_param(SYNTH_PARAMETER param) const {
  if(param < params.size()){
    return &params[param];
  }

  return nullptr;
}

void Synth::run_events(std::vector<Keyboard_Command>& commands){
  for(size_t i = 0; i < commands.size(); i++){
    switch(commands[i].type){
      default: break;
      
      case Keyboard_Command::pitch_bend: {
        f32 bend = calculate_pitch_bend(TWO_SEMITONE_CENTS, normalize_msg_bipolar(commands[i].input.msg2));
        set_pitch_bend(bend);
      } break;

      case Keyboard_Command::note_on: {
        loop_voicings_on(commands[i].input.msg1, normalize_msg(commands[i].input.msg2));
      } break;
      
      case Keyboard_Command::note_off: {
        loop_voicings_off(commands[i].input.msg1);
      } break;
      
      case Keyboard_Command::mod_wheel: {
        set_vibrato_depth(map_vibrato_depth(normalize_msg(commands[i].input.msg2)));
      } break;
      //not impl
      case Keyboard_Command::vol_knob: {
      } break;
    }
  }
}

std::vector<Keyboard_Command> Synth::read_event(Controller& cont) {
  cont.clear_msg_buf();
  std::vector<Keyboard_Command> commands(0);

  const i32 event_count = cont.read_input();
  for(i32 i = 0; i < event_count; i++){
    const PmEvent *ev = cont.get_event_at(i);
    if(!ev){
      continue;
    }
    Midi_Input_Msg msg = cont.parse_event(*ev);
    
    switch(msg.status){
      case CONTROL:{
        switch(msg.msg1){
          case CONTROL_MOD_WHEEL:{
            commands.push_back({ Keyboard_Command::mod_wheel, msg });
          } break;
        }
      }break;

      case PITCH_BEND:{
        commands.push_back({ Keyboard_Command::pitch_bend, msg });
      } break;

      case NOTE_ON: {
        commands.push_back({ Keyboard_Command::note_on, msg });
      }break;
      case NOTE_OFF: {
        commands.push_back({ Keyboard_Command::note_off, msg });
      }break;
    }
  }

  return commands;
}

f32 Synth::calculate_pitch_bend(f32 cents, f32 normalized_midi_event) const {
  const f32 bend = normalized_midi_event * cents;
  return powf(2.0f, bend * CENTS_TO_OCTAVE);
}

f32 Synth::map_vibrato_depth(f32 normalized_midi_event) const {
  return normalized_midi_event * vibrato_depth;
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

f32 Synth::get_sum_at(size_t pos) const {
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

void Synth::loop_voicings_off(u32 midi_key) {
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

void Synth::loop_voicings_on(u32 midi_key, f32 normalized_velocity) {
  for (size_t i = 0; i < voices.size(); i++) {
    Voice *v = &voices[i];
    if (v->get_active_count() <= 0 && v->done()) {
      v->set_active_count(0);
      v->set_key(midi_key);
      v->set_freq(midi_to_freq((i32)midi_key));
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
