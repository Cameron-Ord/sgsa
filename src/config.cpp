#include "audio.hpp"
#include <unordered_map>

const std::unordered_map<std::string, WAVEFORM_TYPE> table_id_map = {
    {"SAW", WAVEFORM_TYPE::SAW},
    {"SINE", WAVEFORM_TYPE::SINE},
};

const std::unordered_map<std::string, ENV_TYPE> env_id_map = {
    {"ADSR", ENV_TYPE::ADSR}, {"AR", ENV_TYPE::AR}};

const std::unordered_map<std::string, LFO_TYPE> lfo_id_map = {
    {"TREMOLO", LFO_TYPE::TREMOLO}, {"VIBRATO", LFO_TYPE::VIBRATO}};
