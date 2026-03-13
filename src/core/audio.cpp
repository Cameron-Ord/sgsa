#include "../../inc/audio.hpp"
#include <cmath>
#include <iostream>

static bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len);
static void generate_loop(Synth *syn, size_t count, f32 *sample_buffer);
static void delay_loop(Synth *syn, size_t count, f32 *sample_buffer);
static void voice_loop(Synth *syn);

const i32 CHUNK_MAX = 128;
const f32 DELAY_MIX = 0.2f;
const f32 SAMPLE_MIX = 0.8f;


//Source: DAFX page 127
f32 exp_hard_clip(f32 sample, f32 gain, f32 mix){
  f32 q = sample * gain;
  f32 z = copysignf(1.0f - expf(-fabsf(q)), q);
  return mix * z + (1.0f - mix) * sample;
}

//Source: DAFX page 125
f32 polynomial_soft_clip(f32 sample, f32 gain){
  f32 threshold = 1.0f / 3.0f;
  f32 x = sample * gain;
  f32 y = 0.0f;
  if(fabsf(x) < threshold){
    y = 2.0f * x;
  }

  if(fabsf(x) >= threshold){
    if(x > 0.0f){
      y = (3.0f - powf(2.0f - x * 3.0f, 2.0f)) / 3.0f;
    } else {
      y = -(3.0f - powf(2.0f - fabsf(x) * 3.0f, 2.0f)) / 3.0f;
    }
  }

  if(fabsf(x) > 2.0f * threshold){
    if(x > 0.0f) {
      y = 1.0f;
    }

    if(x < 0.0f){
      y = -1.0f;
    }
  }

  return y;
}

void stream_get(void *data, SDL_AudioStream *stream, i32 add, i32 total) {
  Synth *syn = static_cast<Synth *>(data);
  if (!syn)
    return;
  // Additional is consumed immediately
  (void)total;
  size_t sample_count = (u32)add / sizeof(f32);
  while (sample_count > 0) {
    f32 samples[CHUNK_MAX];
    memset(samples, 0, sizeof(f32) * CHUNK_MAX);
    const size_t actual_samples = SDL_min(sample_count, SDL_arraysize(samples));
    generate_loop(syn, actual_samples, samples);
    delay_loop(syn, actual_samples, samples);
    stream_feed(stream, samples, (i32)actual_samples * (i32)sizeof(f32));
    sample_count -= actual_samples;
  }
  return;
}

static void delay_loop(Synth *syn, size_t count, f32 *sample_buffer){
  for(size_t i = 0; i < count; i++){
    f32 delayed = syn->get_delay().delay_read();
    const f32 mixed = SAMPLE_MIX * sample_buffer[i] + DELAY_MIX * tanhf(sample_buffer[i] + delayed);
    sample_buffer[i] = mixed;
    syn->get_delay().delay_write(sample_buffer[i]);
  }
}

static void voice_loop(Synth *syn) {
  syn->zero_loop_sums();
  const Synth_Cfg &p = syn->get_synth_cfg();
  const Envelope_Cfg &e = syn->get_env_cfg();

  for (size_t i = 0; i < (size_t)p.voicings && i < SIZES::VOICES; i++) {
    Voice &v = syn->get_voices()[i];
    if (v.get_active_count() <= 0 && !v.releasing()) {
      continue;
    }
    
    v.zero_voice_sums();
    Freq_Modulator& m = v.get_fmod();
    Amp_Modulator& a = v.get_amod();

    m.get_lfo().increment(m.get_vibrato_rate(), p.sample_rate);
    const f32 vib = m.create_vibrato(m.get_lfo().lfo_sine(), m.get_vibrato_depth());

    a.get_lfo().increment(a.get_trem_rate(), p.sample_rate);
    const f32 trem = 1.0f + (a.get_lfo().lfo_sine() * a.get_trem_depth());

    for (size_t o = 0; o < syn->get_osc_count(); o++) {
      Oscillator *osc = v.get_osc_at(o);
      const Oscillator_Cfg *osc_cfg = syn->get_osc_cfg_at(o);

      if(!osc || !osc_cfg) {
        continue;
      }
      const f32 freq = v.get_freq() * osc_cfg->detune * m.get_pitch_bend() * vib; 

      const f32 dt = 1.0f / (f32)p.sample_rate;
      const f32 inc = freq / (f32)p.sample_rate;

      f32 alpha = 1.0f - expf(-dt / 0.001f);
      osc->get_inc() += (inc - osc->get_inc()) * alpha;

      osc->increment_time(dt);
      osc->increment_phase(1.0f);

      for (size_t c = 0; c < (size_t)p.channels; c++) {
        f32 sample = 0.0f;
        switch(osc_cfg->waveform){
          case SAW:{
            sample = syn->get_generator().poly_saw(osc->get_inc(), osc->get_phase_val());
          }break;
          case SQUARE:{
            sample = syn->get_generator().poly_square(osc->get_inc(), osc->get_phase_val(), osc_cfg->duty);
          }break;
        }
        osc->set_sample_at(c, sample);
        v.add_sum_at(c, osc->get_sample_at(c));
      }
    }
    
    // saturate (make optional at some point)
    for(size_t c = 0; c < (size_t)p.channels; c++){
      v.set_clipped_at(c, polynomial_soft_clip(v.get_sum_at(c), p.gain));
    }

    //filter
    for (size_t c = 0; c < (size_t)p.channels; c++) {
      v.get_lpf().lerp(v.get_clipped_array(), c);
      v.set_filtered_at(c, v.get_lpf().get_value_at(c));
    }

    v.adsr(p.sample_rate, e.env_attack, e.env_decay, e.env_sustain,
           e.env_release);

    // Apply amplitude scalars and scale per OSC
    for(size_t c = 0; c < (size_t)p.channels; c++){
      const f32 sample = v.get_filtered_at(c);
      const f32 mixed = sample * trem * v.get_vol_mult() * v.get_envelope();
      const f32 scaled = mixed * (1.0f / sqrtf((f32)syn->get_osc_count()));
      v.set_out_at(c, scaled);
    }

    // Add scaled sum of osc and scale per VOICE
    for (size_t c = 0; c < (size_t)p.channels; c++) {
      const f32 sample = v.get_out_at(c);
      const f32 scaled = sample * (1.0f / sqrtf(((f32)p.voicings)));
      syn->add_sum_at(c, scaled);
    }
  }
}

static void generate_loop(Synth *syn, size_t count, f32 *sample_buffer) {
  const size_t channels = static_cast<size_t>(syn->get_synth_cfg().channels);
  const size_t N = static_cast<size_t>(count / channels);
  for (size_t n = 0; n < N; n++) {
    voice_loop(syn);
    for (size_t c = 0; c < channels; c++) {
      sample_buffer[n * channels + c] = syn->get_sum_at(c);
    }
  }
}

static bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len) {
  return SDL_PutAudioStreamData(stream, samples, len);
}

Audio_Sys::Audio_Sys(i32 chan, i32 sample_rate)
    : dev(0), stream(NULL), internal({SDL_AUDIO_F32, chan, sample_rate}),
      output({SDL_AUDIO_F32, 0, 0}) {}

bool Audio_Sys::open(void *userdata) {
  if (!(open_audio_device() && create_audio_stream())) {
    return false;
  }

  if (!(set_audio_callback(userdata) && bind_stream())) {
    return false;
  }
  return true;
}

void Audio_Sys::close(void) {
  pause();
  clear();
  unbind_stream();
  destroy_audio_stream();
  close_audio_device();
}

void Audio_Sys::clear(void) {
  if (!SDL_ClearAudioStream(stream)) {
    std::cerr << "Failed to clear stream: " << SDL_GetError() << std::endl;
  }
}

bool Audio_Sys::set_audio_callback(void *userdata) {
  if (!SDL_SetAudioStreamGetCallback(stream, stream_get, userdata)) {
    std::cerr << "Failed to set callback: " << SDL_GetError() << std::endl;
    return false;
  }
  return true;
}

bool Audio_Sys::bind_stream(void) {
  if (!(stream && dev)) {
    std::cerr << "Invalid parameters" << std::endl;
    return false;
  }

  if (!SDL_BindAudioStream(dev, stream)) {
    std::cerr << SDL_GetError() << std::endl;
    return false;
  }

  return true;
}

bool Audio_Sys::unbind_stream(void) {
  if (!stream) {
    std::cerr << "Invalid parameters" << std::endl;
    return false;
  }

  SDL_UnbindAudioStream(stream);
  return true;
}

bool Audio_Sys::open_audio_device(void) {
  dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
  if (!dev) {
    std::cerr << SDL_GetError() << std::endl;
    return false;
  }
  SDL_GetAudioDeviceFormat(dev, &output, NULL);
  const char *name = SDL_GetAudioDeviceName(dev);
  std::cout << "Device name: " << name << std::endl;
  std::cout << "Channels: " << output.channels << " Samplerate: " << output.freq
            << std::endl;
  return true;
}

bool Audio_Sys::create_audio_stream(void) {
  stream = SDL_CreateAudioStream(&internal, &output);
  if (!stream) {
    std::cerr << SDL_GetError() << std::endl;
    return false;
  }
  return true;
}

bool Audio_Sys::close_audio_device(void) {
  if (!dev) {
    std::cerr << "Invalid parameter" << std::endl;
    return false;
  }
  SDL_CloseAudioDevice(dev);
  return true;
}

bool Audio_Sys::destroy_audio_stream(void) {
  if (!stream) {
    std::cerr << "Invalid parameter" << std::endl;
    return false;
  }
  return true;
}

bool Audio_Sys::resume(void) {
  if (!dev) {
    std::cerr << "Invalid parameter" << std::endl;
    return false;
  }

  if (!SDL_PauseAudioDevice(dev)) {
    std::cerr << SDL_GetError() << std::endl;
    return false;
  }

  return true;
}

bool Audio_Sys::pause(void) {
  if (!dev) {
    std::cerr << "Invalid parameter" << std::endl;
    return false;
  }

  if (!SDL_ResumeAudioDevice(dev)) {
    std::cerr << SDL_GetError() << std::endl;
    return false;
  }

  return true;
}
