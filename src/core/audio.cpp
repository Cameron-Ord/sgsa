#include "../../inc/audio.hpp"
#include <cmath>
#include <iostream>

static bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len);
static void generate_loop(Synth *syn, size_t count, f32 *sample_buffer);
static void delay_loop(Synth *syn, size_t count, f32 *sample_buffer);
static f32 waveform_generate(const Synth *syn, size_t osc_index, f32 phase, f32 freq);
static void voice_loop(Synth *syn, f32 generated[CHANNEL_MAX]);

const i32 CHUNK_MAX = 128;
const f32 DELAY_MIX = 0.2f;
const f32 SAMPLE_MIX = 0.8f;

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

static f32 waveform_generate(const Synth *syn, size_t osc_index, f32 phase,
                    f32 freq) {
  if(osc_index > syn->get_osc_cfgs().size()){
    return 0.0f;
  }

  const size_t octave_index = syn->get_wave_table().index_octave(freq);
  if (const Waveform_Vec4f* table = syn->get_wave_table().get_table()) {
    const size_t base = (size_t)floorf(phase);
    
    size_t j = base % syn->get_synth_cfg().wave_table_size;
    size_t k = (j + 1) % syn->get_synth_cfg().wave_table_size;
    f32 f = phase - (f32)base;

    const f32 *jval = table->get_at(syn->get_osc_cfgs()[osc_index].waveform, osc_index, octave_index, j);    
    const f32 *kval = table->get_at(syn->get_osc_cfgs()[osc_index].waveform, osc_index, octave_index, k);    

    if(!jval || !kval) { 
      return 0.0f;
    }

    return *jval + ((*kval - *jval) * f);
  }
  return 0.0f;
}

static void voice_loop(Synth *syn, f32 generated[SIZES::CHANNEL_MAX]) {
  syn->zero_loop_sums();
  const Synth_Cfg &p = syn->get_synth_cfg();
  const Envelope_Cfg &e = syn->get_env_cfg();

  for (size_t i = 0; i < (size_t)p.voicings && i < SIZES::VOICES; i++) {
    Voice &v = syn->get_voices()[i];
    if (v.get_active_count() <= 0 && !v.releasing()) {
      continue;
    }
    
    v.zero_voice_sums();
    for (size_t o = 0; o < syn->get_osc_count(); o++) {
      Oscillator *osc = v.get_osc_at(o);
      const Oscillator_Cfg *osc_cfg = syn->get_osc_cfg_at(o);

      if(!osc || !osc_cfg) {
        continue;
      }

      const f32 freq = v.get_freq() * osc_cfg->detune; 

      const f32 dt = 1.0f / (f32)p.sample_rate;
      const f32 inc = (f32)p.wave_table_size * freq / (f32)p.sample_rate;

      osc->increment_time(dt);
      osc->increment_phase(inc, (f32)p.wave_table_size);

      for (size_t c = 0; c < (size_t)p.channels; c++) {
        const f32 sample = waveform_generate(syn, o, osc->get_phase_val(), freq);
        osc->set_sample_at(c, sample);
        osc->mult_sample_at(c, osc_cfg->volume);
        osc->mult_sample_at(c, 1.0f/ sqrtf((f32)syn->get_osc_count()));
        v.add_sum_at(c, osc->get_sample_at(c));
      }
    }

    for (size_t c = 0; c < (size_t)p.channels; c++) {
      v.get_sum_array()[c] = tanhf(v.get_sum_at(c) * p.gain);
      v.get_lpf().lerp(v.get_sum_array(), c);
    }
    v.adsr(p.sample_rate, e.env_attack, e.env_decay, e.env_sustain,
           e.env_release);
    for (size_t c = 0; c < (size_t)p.channels; c++) {
      // Saturate
      const f32 mix = v.get_lpf().get_value_at(c);
      syn->add_sum_at(c, (mix / sqrtf((f32)p.voicings)) * p.volume * v.get_envelope());
    }
  }

  for (size_t c = 0; c < (size_t)p.channels; c++) {
    generated[c] = syn->get_sum_at(c);
  }
}

static void generate_loop(Synth *syn, size_t count, f32 *sample_buffer) {
  for (i32 n = 0; n < (i32)count / syn->get_synth_cfg().channels; n++) {
    f32 generated[SIZES::CHANNEL_MAX] = {0.0f, 0.0f};
    voice_loop(syn, generated);
    for (i32 c = 0; c < syn->get_synth_cfg().channels; c++) {
      sample_buffer[n * syn->get_synth_cfg().channels + c] = generated[c];
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
