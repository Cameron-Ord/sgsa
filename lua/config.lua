local waves = { SAW = 0, SINE = 1 }
local modes = { TREMOLO = 0, VIBRATO = 1 }
return {
  lfo_on = false,
  lfo_rate = 1.0,
  lfo_depth = 1.0,
  lfo_timer = 0.3,
  lfo_mode = modes["TREMOLO"],
  env_attack = 0.1,
  env_decay = 0.1,
  env_sustain = 0.1,
  env_release = 0.1,
  volume = 1.0,
  gain = 1.0,
  channels = 2,
  sample_rate = 48000,
  voicings = 8,
  wave_table_size = 512,
  tempo = 120.0,
  note_duration = 1.0,
  filter_cutoff_low = 40.0,
  filter_cutoff_high = 20000.0,
  use_filter = true,
  oscilators = {
    {
      duty_cycle = 0.5,
      detune = 1.0,
      volume = 1.0,
      octave_step = 1.0,
      waveform = waves["SAW"]
    },
  },
}
