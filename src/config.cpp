#include "config.hpp"
#include <iostream>

void Synth_Cfg::print(void) const {
  std::cout << "====================" << std::endl;
  std::cout << "lfo rate: (" << lfo_rate << "), "
            << "lfo depth: (" << lfo_depth << "), "
            << "lfo timer: (" << lfo_timer << ") " << std::endl;

  std::cout << "volume: (" << volume << "), "
            << "gain: (" << gain << "), "
            << "filter cutoff: (" << low_pass_cutoff << "), "
            << "attack: (" << env_attack << "), "
            << "decay: (" << env_decay << "), "
            << "sustain: (" << env_sustain << "), "
            << "release: (" << env_release << ") " << std::endl;

  std::cout << "channels: (" << channels << "), "
            << "sample rate: (" << sample_rate << "), "
            << "lfo mode: (" << lfo_mode << "), "
            << "voicings: (" << voicings << "), "
            << "table size: (" << wave_table_size << ") " << std::endl;
  std::cout << "====================" << std::endl;
}

void Oscilator_Cfg::print(void) const {
  std::cout << "====================" << std::endl;
  std::cout << "detune: (" << detune << "), "
            << "volume: (" << volume << "), "
            << "octave: (" << octave_step << "), "
            << "waveform: (" << waveform << ") " << std::endl;
  std::cout << "====================" << std::endl;
}
