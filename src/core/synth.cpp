#include "../../inc/synth.hpp"
#include "../../inc/util.hpp"
#include <cmath>

const size_t DEFAULT_OSC_COUNT = 1;
const f32 DEFAULT_DELAY_TIME = 0.5f;
const f32 DEFAULT_DELAY_FEEDBACK = 0.5f;

const f32 MINUTE = 60.0f;
const f32 BPM_MIN = 1.0f;
const f32 BPM_MAX = 240.0f;
const f32 BASE_BPM = 120.0f;

const f32 WHOLE_NOTE_BEATS = 4.0f;
const f32 WHOLE_NOTE = 1.0f;
const f32 HALF_NOTE = WHOLE_NOTE / 2.0f;
const f32 QUARTER_NOTE = WHOLE_NOTE / 4.0f;
const f32 EIGHT_NOTE = WHOLE_NOTE / 8.0f;
const f32 SIXTEENTH_NOTE = WHOLE_NOTE / 16.0f;

const f32 ENV_DEFAULT = 0.1f;
const f32 ENV_MAX = 2.0f;
const f32 ENV_MIN = 0.01f;

const f32 LPF_MIN = 25.0f;
const f32 LPF_MAX = 10000.0f;
const f32 LPF_DEFAULT = 1000.0f;

const f32 TREM_MAX = 1.0f;
const f32 TREM_MIN = 0.0f;
const f32 TREM_DEFAULT = 0.25f;

const f32 DELAY_MAX = 3.0f;
const f32 DELAY_MIN = 0.1f;
const f32 DELAY_DEFAULT = 0.25f;

const f32 GAIN_MAX = 6.0f;
const f32 GAIN_MIN = 0.5f;
const f32 GAIN_DEFAULT = 1.0f;

const f32 VOL_MIN = 0.0f;
const f32 VOL_MAX = 1.0f;
const f32 VOL_DEFAULT = 1.0f;

f32 create_vibrato(f32 sine, f32 cents) {
  return powf(2.0f, sine * cents * CENTS_TO_OCTAVE);
}

void lerp_f32(const f32 *target, f32 *val, const f32 alpha) {
  if (!val || !target)
    return;
  *val = *val + (*target - *val) * alpha;
}

f32 time_to_hz(f32 time) { return 1.0f / time; }

f32 hz_to_rad_per_sec(f32 hz) { return 2.0f * PI * hz; }

f32 rad_per_sec_to_hz(f32 rad) { return rad / (2.0f * PI); }

f32 note_to_time(f32 tempo, f32 note_time) {
  return (MINUTE / tempo) * (note_time * WHOLE_NOTE_BEATS);
}

Synth::Synth(void)
    : params_f32(init_params()),
      note_durations(
          {WHOLE_NOTE, HALF_NOTE, QUARTER_NOTE, EIGHT_NOTE, SIXTEENTH_NOTE}),
      oscs(DEFAULT_OSC_COUNT), voices(), generator(),
      delay(sample_rate, DEFAULT_DELAY_TIME, DEFAULT_DELAY_FEEDBACK),
      loop_sums() {}

// attack - decay - sustain - release

std::array<ParamF32, S_PARAM_COUNT> Synth::init_params(void) {
  // MIN - MAX - VALUE - INC
  return {
      ParamF32("Attack", ENV_MIN, ENV_MAX, ENV_DEFAULT),
      ParamF32("Decay", ENV_MIN, ENV_MAX, ENV_DEFAULT),
      ParamF32("Sustain", ENV_MIN, ENV_MAX, ENV_DEFAULT),
      ParamF32("Release", ENV_MIN, ENV_MAX, ENV_DEFAULT),

      ParamF32("Volume", VOL_MIN, VOL_MAX, VOL_DEFAULT),
      ParamF32("Gain", GAIN_MIN, GAIN_MAX, GAIN_DEFAULT),
      ParamF32("Low-Pass", LPF_MIN, LPF_MAX, LPF_DEFAULT),
      ParamF32("Tremolo", TREM_MIN, TREM_MAX, TREM_DEFAULT),
      ParamF32("Delay Time", DELAY_MIN, DELAY_MAX, DELAY_DEFAULT),

      ParamF32("BPM", BPM_MIN, BPM_MAX, BASE_BPM),
  };
}

// Source: DAFX page 127
f32 Synth::exp_hard_clip(const f32 *sample, f32 gain, f32 mix) const {
  if (!sample)
    return 0.0f;

  f32 q = *sample * gain;
  f32 z = copysignf(1.0f - expf(-fabsf(q)), q);
  return mix * z + (1.0f - mix) * *sample;
}

// Source: DAFX page 125
f32 Synth::polynomial_soft_clip(const f32 *sample, f32 gain) const {
  if (!sample)
    return 0.0f;

  f32 threshold = 1.0f / 3.0f;
  f32 x = *sample * gain;
  f32 y = 0.0f;
  if (fabsf(x) < threshold) {
    y = 2.0f * x;
  }

  if (fabsf(x) >= threshold) {
    if (x > 0.0f) {
      y = (3.0f - powf(2.0f - x * 3.0f, 2.0f)) / 3.0f;
    } else {
      y = -(3.0f - powf(2.0f - fabsf(x) * 3.0f, 2.0f)) / 3.0f;
    }
  }

  if (fabsf(x) > 2.0f * threshold) {
    if (x > 0.0f) {
      y = 1.0f;
    }

    if (x < 0.0f) {
      y = -1.0f;
    }
  }

  return y;
}

void Synth::inc_param(SYNTH_PARAMETER param) {
  if (param < params_f32.size()) {
    // const f32 min = params_f32[param].min;
    // const f32 max = params_f32[param].max;
    // const f32 base_val = params_f32[param].value;
    // const f32 inc_val = params_f32[param].inc;
    //
    // params_f32[param].value = clamp_param_f32(min, max, base_val + inc_val);
    // if(param == S_DELAY_TIME){
    // delay.rebuild(sample_rate, params_f32[param].value);
    //}
  }
}

void Synth::dec_param(SYNTH_PARAMETER param) {
  if (param < params_f32.size()) {
    // const f32 min = params_f32[param].min;
    // const f32 max = params_f32[param].max;
    // const f32 base_val = params_f32[param].value;
    //
    // params_f32[param].value = clamp_param_f32(min, max, base_val + inc_val);
    // if(param == S_DELAY_TIME){
    // delay.rebuild(sample_rate, params_f32[param].value);
    //}
  }
}

f32 Synth::clamp_param_f32(f32 min, f32 max, f32 value) {
  if (value < min) {
    value = min;
  }

  if (value > max) {
    value = max;
  }

  return value;
}

void Synth::set_param(SYNTH_PARAMETER param, f32 value) {
  if (param < params_f32.size()) {
    params_f32[param].value =
        clamp_param_f32(params_f32[param].min, params_f32[param].max, value);
  }
}

ParamF32 *Synth::get_param(SYNTH_PARAMETER param) {
  if (param < params_f32.size()) {
    return &params_f32[param];
  }

  return nullptr;
}

const ParamF32 *Synth::get_param(SYNTH_PARAMETER param) const {
  if (param < params_f32.size()) {
    return &params_f32[param];
  }

  return nullptr;
}

void Synth::run_events(std::vector<Keyboard_Command> &commands) {
  for (size_t i = 0; i < commands.size(); i++) {
    switch (commands[i].type) {
    default:
      break;

    case Keyboard_Command::pitch_bend: {
      f32 bend = calculate_pitch_bend(
          TWO_SEMITONE_CENTS, normalize_msg_bipolar(commands[i].input.msg2));
      set_pitch_bend(bend);
    } break;

    case Keyboard_Command::note_on: {
      loop_voicings_on(commands[i].input.msg1,
                       normalize_msg(commands[i].input.msg2));
    } break;

    case Keyboard_Command::note_off: {
      loop_voicings_off(commands[i].input.msg1);
    } break;

    case Keyboard_Command::mod_wheel: {
      set_vibrato_depth(
          map_vibrato_depth(normalize_msg(commands[i].input.msg2)));
    } break;
    // not impl
    case Keyboard_Command::vol_knob: {
    } break;
    }
  }
}

std::vector<Keyboard_Command> Synth::read_event(Controller &cont) {
  cont.clear_msg_buf();
  std::vector<Keyboard_Command> commands(0);

  const i32 event_count = cont.read_input();
  for (i32 i = 0; i < event_count; i++) {
    const PmEvent *ev = cont.get_event_at(i);
    if (!ev) {
      continue;
    }
    Midi_Input_Msg msg = cont.parse_event(*ev);

    switch (msg.status) {
    case CONTROL: {
      switch (msg.msg1) {
      case CONTROL_MOD_WHEEL: {
        commands.push_back({Keyboard_Command::mod_wheel, msg});
      } break;
      }
    } break;

    case PITCH_BEND: {
      commands.push_back({Keyboard_Command::pitch_bend, msg});
    } break;

    case NOTE_ON: {
      commands.push_back({Keyboard_Command::note_on, msg});
    } break;
    case NOTE_OFF: {
      commands.push_back({Keyboard_Command::note_off, msg});
    } break;
    }
  }

  return commands;
}

f32 Synth::calculate_pitch_bend(f32 cents, f32 normalized_midi_event) const {
  const f32 bend = normalized_midi_event * cents;
  return powf(2.0f, bend * CENTS_TO_OCTAVE);
}

f32 Synth::map_vibrato_depth(f32 normalized_midi_event) const {
  return normalized_midi_event * vibrato_max;
}

void Synth::zero_loop_sums(void) {
  for (size_t i = 0; i < loop_sums.size(); i++) {
    loop_sums[i] = 0.0f;
  }
}

Oscillator *Synth::get_osc_at(size_t pos) {
  if (pos < oscs.size()) {
    return &oscs[pos];
  }
  return nullptr;
}

f32 Synth::get_sum_at(size_t pos) const {
  if (pos < loop_sums.size()) {
    return loop_sums[pos];
  }
  return 0.0f;
}

void Synth::add_sum_at(size_t pos, const f32 *sum) {
  if (!sum) {
    return;
  }
  if (pos < loop_sums.size()) {
    loop_sums[pos] = loop_sums[pos] + *sum;
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

      for (size_t c = 0; c < static_cast<size_t>(channels); c++) {
        v->set_clipped_at(c, 0.0f);
        v->set_filtered_at(c, 0.0f);
        v->set_out_at(c, 0.0f);
      }
      return;
    }
  }
}
