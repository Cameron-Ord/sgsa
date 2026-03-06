#include "audio.hpp"
#include <cmath>
#include <iostream>

static bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len);
static void generate_loop(Synth *syn, size_t count, f32 *sample_buffer);
static f32 generate(const Wave_Table *wt, f32 phase, size_t table_id, f32 freq);
static void voice_loop(Synth *syn, f32 generated[CHANNEL_MAX]);

const i32 CHUNK_MAX = 128;

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
    stream_feed(stream, samples, (i32)actual_samples * (i32)sizeof(f32));
    sample_count -= actual_samples;
  }
  return;
}

static f32 generate(const Wave_Table *wt, f32 phase, size_t table_id,
                    f32 freq) {
  const size_t i = wt->index_octave(freq);
  if (const f32 *wave = wt->get_table(table_id, i)) {
    size_t j = (size_t)floorf(phase) % wt->get_size();
    size_t k = (j + 1) % wt->get_size();
    f32 f = phase - (f32)j;
    return wave[j] + ((wave[k] - wave[j]) * f);
  }
  return 0.0f;
}

static void voice_loop(Synth *syn, f32 generated[SIZES::CHANNEL_MAX]) {
  f32 sums[SIZES::CHANNEL_MAX] = {0.0f, 0.0f};
  const Synth_Cfg &p = syn->get_cfg();

  for (size_t i = 0; i < (size_t)p.voicings && i < SIZES::VOICES; i++) {
    Voice &v = syn->get_voices()[i];
    if (!(v.get_active_count() > 0) && !v.releasing()) {
      continue;
    }

    f32 voice_sums[SIZES::CHANNEL_MAX] = {0.0f, 0.0f};
    for (size_t o = 0; o < v.get_osc_count(); o++) {
      Oscilator &osc = v.get_osc_array()[o];

      const size_t wt_size = (size_t)p.wave_table_size;
      const f32 freq =
          v.get_freq() * osc.get_cfg().detune * osc.get_cfg().octave_step;
      const f32 dt = 1.0f / (f32)p.sample_rate;
      const f32 inc = (f32)wt_size * freq / (f32)p.sample_rate;

      osc.increment_time(dt);
      osc.increment_phase(inc, (f32)wt_size);

      for (i32 c = 0; c < p.channels; c++) {
        osc.get_sample_array()[c] =
            generate(&syn->get_wave_table(), osc.get_phase_val(),
                     (size_t)osc.get_cfg().waveform, freq);
        osc.get_sample_array()[c] *= osc.get_cfg().volume;
        osc.get_sample_array()[c] /= sqrtf((f32)v.get_osc_count());
        voice_sums[c] += osc.get_sample_array()[c];
      }
    }

    for (i32 c = 0; c < p.channels; c++) {
      // Saturate
      voice_sums[c] = tanhf(voice_sums[c] * p.gain);
      v.get_lpf().lerp(voice_sums, c);

      const f32 mix = v.get_lpf().get_array()[c];

      v.adsr(p.sample_rate, p.env_attack, p.env_decay, p.env_sustain,
             p.env_release);

      sums[c] += (mix / sqrtf((f32)p.voicings)) * p.volume * v.get_envelope();
    }
  }

  for (i32 c = 0; c < p.channels; c++) {
    generated[c] = sums[c];
  }
}

static void generate_loop(Synth *syn, size_t count, f32 *sample_buffer) {
  for (i32 n = 0; n < (i32)count / syn->get_cfg().channels; n++) {
    f32 generated[SIZES::CHANNEL_MAX] = {0.0f, 0.0f};
    voice_loop(syn, generated);
    for (i32 c = 0; c < syn->get_cfg().channels; c++) {
      sample_buffer[n * syn->get_cfg().channels + c] = generated[c];
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
