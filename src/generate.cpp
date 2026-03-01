#include "sgsa.hpp"
#include "util.hpp"
#include <iostream>
#include <cmath>

// ==WAVE TABLE BEGIN==
void Wave_Table::print_table(f32 table[MAX_TABLE_SIZE]){
    for(size_t i = 0; i < size && i < MAX_TABLE_SIZE; i++){
        std::cout << "(" << table[i] << ")";
    }
    std::cout << std::endl;
}

u8 Wave_Table::index_octave(f32 freq) const {
    for(u8 i = 0; i < OCTAVES - 1; i++){
        const f32 base = freq_mapper[i];
        const f32 next = freq_mapper[i + 1];
        if(freq >= base && freq < next) {
            return i;
        }
    }
    return OCTAVES - 1;
}

Wave_Table::Wave_Table(i32 sample_rate, size_t table_size) 
  : tables(), freq_mapper(), size(table_size) 
{
    const size_t N = size;
    // C0 lowest note on the piano realistically possible
    const f32 C0 = 16.35f;

    for(size_t o = 0; o < OCTAVES; o++){
        // Calculate the base of the current octave
        const f32 freq = C0 * powf(2.0f, (f32)o);
        // Find the end of the current harmonic content of this octave 
        // by dividing the nyquist by the top of the current octave
        const i32 end = (i32)(NYQUIST((f32)sample_rate) / (freq * 2.0f));
        freq_mapper[o] = freq * 2.0f;

        for(size_t n = 0; n < N; n++){
            const f32 phase = (f32)n / (f32)N;
            f32 sum = 0.0f;
            for(i32 k = 1; k <= end; k++){
                f32 sign = powf(-1.0f, (f32)k);
                sum += sign * sinf(2.0f * PI * (f32)k * phase) / (f32)k;
            }
            tables[TABLE_SAW][o][n] = -((2.0f * 1.0f) / PI) * sum;
        }

        for(size_t n = 0; n < N; n++){
          const f32 phase = (f32)n / (f32)N;
          tables[TABLE_SINE][o][n] = sinf(2.0f * PI * phase);
        }
    }
}
// ==WAVE TABLE END==
// ==VOICE BEGIN==
Voice::Voice(i32 sr, size_t osc_c, const Env_Params& envp, const Lfo_Params& lfop, std::vector<Oscilator_Cfg> templates) 
  : sample_rate(sr), env_(envp), lfop_(lfop), active_oscilators(0), freq(0.0f), midi_key(0),
    osc_count(osc_c), cfgs(templates), oscs(osc_c, Oscilator(sr, envp, lfop))
{}
// ==VOICE END==
// ==AUDIO DATA BEGIN==
Audio_Data::Audio_Data(const Params& p, std::vector<Oscilator_Cfg> templates, size_t osc_c) 
  : lfop_(p.lfop), ap_(p.ap), envp_(p.envp), 
  voices(p.ap.voicings, Voice(p.ap.sample_rate, osc_c, p.envp, p.lfop, templates)), wave_table(p.ap.sample_rate, p.ap.wave_table_size) {}
// ==AUDIO DATA END==
// ==LFO BEGIN==
Lfo::Lfo(f32 r, f32 d, f32 t, i32 sr, u8 m)
  : mode(m), rate(r), depth(d), timer(t), inc(r / (f32)sr), max(1.0f), phase(0.0f){}

f32 Lfo::vibrato(void){
    return depth * sinf(2.0f * PI * phase);
}

void Lfo::increment_lfo(void){
  phase += inc;
  if(phase > max){
    phase -= max;
  }
}
// ==LFO END==
// ==OSCILATOR BEGIN==
Oscilator::Oscilator(i32 sr, const Env_Params& env, const Lfo_Params& lfop)
  : env_(env), lfop_(lfop), lfo(lfop.rate, lfop.depth, lfop.timer, sr, lfop.mode)
{
  memset(gen_states, 0, sizeof(f32) * STATE_END);
  samples = Interpolator();
  env_state = ENVELOPE_OFF;
}

void Oscilator::increment_time(f32 inc){
    gen_states[STATE_TIME] += inc;
}

void Oscilator::increment_phase(f32 inc, f32 max){
    gen_states[STATE_PHASE] += inc;
    if(gen_states[STATE_PHASE] >= max){
        gen_states[STATE_PHASE] -= max;
    }
}

void Oscilator::adsr(i32& counter, i32 samplerate, f32 atk, f32 dec, f32 sus, f32 rel){
    switch(env_state){
        default: return;
        case ENVELOPE_ATTACKING: {
            gen_states[STATE_ENVELOPE] += ATTACK_INCREMENT((f32)samplerate, atk);
            if(gen_states[STATE_ENVELOPE] >= 1.0f){
                gen_states[STATE_ENVELOPE] = 1.0f;
                env_state = ENVELOPE_DECAYING;
            }
        } break;

        case ENVELOPE_DECAYING: {
            gen_states[STATE_ENVELOPE] -= DECAY_INCREMENT((f32)samplerate, dec, sus);
            if (gen_states[STATE_ENVELOPE] <= sus) {
                gen_states[STATE_ENVELOPE] = sus;
                env_state = ENVELOPE_SUSTAINING;
            }       
        }break;

        case ENVELOPE_RELEASING: {
            gen_states[STATE_ENVELOPE] -= RELEASE_INCREMENT((f32)samplerate, rel);
            if (gen_states[STATE_ENVELOPE] <= 0.0f) {
                gen_states[STATE_ENVELOPE] = 0.0f;
                env_state = ENVELOPE_OFF;
                counter--;
            }
        }break;
    }
}

void Oscilator::ar(i32& counter, i32 samplerate, f32 atk, f32 rel){
    switch(env_state){
      default: break;
      case ENVELOPE_ATTACKING:{
        gen_states[STATE_ENVELOPE] += ATTACK_INCREMENT((f32)samplerate, atk);
        if(gen_states[STATE_ENVELOPE] >= 1.0f){
            gen_states[STATE_ENVELOPE] = 1.0f;
            env_state = ENVELOPE_RELEASING;
        }
      }break;
      case ENVELOPE_RELEASING:{
        gen_states[STATE_ENVELOPE] -= RELEASE_INCREMENT((f32)samplerate, rel);
        if (gen_states[STATE_ENVELOPE] <= 0.0f) {
            gen_states[STATE_ENVELOPE] = 0.0f;
            env_state = ENVELOPE_OFF;
            counter--;
        }
      }break;
    }
}

// ==OSCILATOR END==
// ==INTERPOLATOR==
Interpolator::Interpolator(void){
  for(size_t c = 0; c < CHANNEL_MAX; c++){
    unfiltered[c] = 0.0f;
    high[c] = 0.0f;
    low[c] = 0.0f;
    filtered[c] = 0.0f;
  }
}

void Interpolator::lerp(f32 alpha_low, f32 alpha_high, i32 c){
  high[c] = high[c] + (unfiltered[c] - high[c]) * alpha_high;
  low[c] = low[c] + (unfiltered[c] - low[c]) * alpha_low;
}
// ==INTERPOLATOR END==



