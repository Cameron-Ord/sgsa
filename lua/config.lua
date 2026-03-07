local waves = { SAW = 0, SINE = 1, SQUARE = 2, TRIANGLE = 3, PULSE = 4 }
local modes = { TREMOLO = 0, VIBRATO = 1 }
return {
  lfo_on = false,
  lfo_rate = 1.0,
  lfo_depth = 1.0,
  lfo_timer = 0.3,
  lfo_mode = modes["TREMOLO"],
  env_attack = 0.005,
  env_decay = 0.18,
  env_sustain = 0.35,
  env_release = 0.25,
  volume = 1.0,
  gain = 2.25,
  channels = 2,
  sample_rate = 48000,
  voicings = 8,
  wave_table_size = 2048,
  tempo = 120.0,
  note_duration = 1.0,
  low_pass_cutoff = 800.0,
  use_filter = true,
  duty_cycle = 0.33,
  oscilators = {
    {
      detune = 1.0,
      volume = 1.0,
      octave_step = 1.0,
      waveform = waves["SAW"]
    },
    {
      detune = 1.002,
      volume = 0.7,
      octave_step = 1.0,
      waveform = waves["SAW"]
    },
    {
      detune = 1.0 - 0.002,
      volume = 0.3,
      octave_step = 1.0,
      waveform = waves["SAW"]
    },
    {
      detune = 1.0,
      volume = 0.6,
      octave_step = 1.0,
      waveform = waves["TRIANGLE"]
    },
  },
}
