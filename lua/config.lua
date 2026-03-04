return {
  lfo = {
    on = false,
    rate = 1.0,
    depth = 1.0,
    timer = 0.3,
    mode = "TREMOLO"
  },
  envelope = {
    attack = 0.1,
    decay = 0.1,
    sustain = 0.1,
    release = 0.1,
    type = "ADSR"
  },
  audio = {
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
    use_filter = true
  },
  oscilators = {
    {
      duty_cycle = 0.5,
      detune = 1.0,
      volume = 1.0,
      octave_step = 1.0,
      waveform = "SAW"
    }
  },
}
