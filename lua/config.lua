local waves = { SAW = 0, SINE = 1 }
local modes = { TREMOLO = 0, VIBRATO = 1 }
return {
  lfo_on = false,
  lfo_rate = 1.0,
  lfo_depth = 1.0,
  lfo_timer = 0.3,
  lfo_mode = modes["TREMOLO"],
  env_attack = 1.0,
  env_decay = 0.8,
  env_sustain = 0.7,
  env_release = 1.0,
  volume = 1.0,
  gain = 2.25,
  channels = 2,
  sample_rate = 48000,
  voicings = 8,
  wave_table_size = 2048,
  tempo = 120.0,
  note_duration = 1.0,
  filter_cutoff_low = 60.0,
  filter_cutoff_high = 14000.0,
  use_filter = true,
  oscilators = {
    {
      duty_cycle = 0.5,
      detune = 1.0,
      volume = 1.0,
      octave_step = 1.0,
      waveform = waves["SAW"]
    },
    {
      duty_cycle = 0.5,
      detune = 0.996,
      volume = 0.6,
      octave_step = 1.0,
      waveform = waves["SAW"]
    },
    {
      duty_cycle = 0.5,
      detune = 1.004,
      volume = 0.6,
      octave_step = 1.0,
      waveform = waves["SAW"]
    },
    {
      duty_cycle = 0.5,
      detune = 1.0,
      volume = 0.9,
      octave_step = 0.5,
      waveform = waves["SINE"]
    },
  },
}
