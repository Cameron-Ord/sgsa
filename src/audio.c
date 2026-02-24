#include "../include/audio.h"

#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "../include/effect.h"
#include "../include/util.h"
#include "../include/waveform.h"
const size_t SAMPLE_PER_CALLBACK = 128;

// make these variables at some point
const f32 DELAY_MIX = 0.3f;
const f32 SAMPLE_MIX = 0.7f;

const f32 HIGH_MIX = 0.3f;
const f32 LOW_MIX = 0.7f;

static void render_push(f32 *samples, size_t nsamples, f32 *buffer,
                        size_t buflen) {
  if (nsamples > 0 && samples) {
    memmove(buffer, buffer + nsamples, (buflen - nsamples) * sizeof(f32));
    memcpy(buffer + (buflen - nsamples), samples, nsamples * sizeof(f32));
  }
}

bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len) {
  return SDL_PutAudioStreamData(stream, samples, len);
}

static void loop_delay(size_t nsamples, f32 *samples, const struct configs *cfg,
                       struct delay_line *dl) {
  for (size_t i = 0; i < nsamples; i++) {
    const f32 delayed =
     delay_line_read(dl) * cfg->fvals[DELAY_FEEDBACK_VAL].value;
    const f32 added =
     tanhf(samples[i] + delayed * cfg->fvals[DELAY_GAIN_VAL].value);
    samples[i] = SAMPLE_MIX * samples[i] + DELAY_MIX * added;
    delay_line_write(samples[i], dl);
  }
}

static bool oscilator_state_on(const bool *active, const i32 *envelope_state) {
  const bool first = *active && (*envelope_state != ENVELOPE_OFF || *envelope_state != ENVELOPE_RELEASE);
  const bool second = !*active && *envelope_state == ENVELOPE_RELEASE;
  return first || second;
}

static f32 set_vibrato(f32 freq, const f32 *state, const struct configs *cfg) {
  if (state[TIME_VAL] > cfg->fvals[VIBRATO_ONSET_VAL].value) {
    const f32 depth = cfg->fvals[VIBRATO_DEPTH_VAL].value;
    const f32 rate = cfg->fvals[VIBRATO_RATE_VAL].value;
    const i32 sr = cfg->ivals[SAMPLE_RATE_VAL].value;
    return vibrato(rate, depth, freq, sr);
  }
  return freq;
}

static f32 generate(f32 ampl, i32 waveform_id, f32 *state,
                    const struct osc_entry_f32 *spec, f32 inc, f32 dc_blocker) {

  const f32 phase = state[PHASE_VAL];
  const f32 vol = spec[OSC_VOLUME_VAL].value;
  const f32 coeff = spec[COEFF_VAL].value;

  f32 *integrator = &state[INTEGRATOR_VAL];
  f32 *dc_y = &state[DC_Y_VAL];
  f32 *dc_x = &state[DC_X_VAL];

  switch (waveform_id) {
  default:
    return 0.0f;
  case SINE: {
    return sine(ampl, phase) * vol;
  } break;
  case PULSE_POLY: {
    return poly_square(ampl, inc, phase, coeff) * vol;
  } break;
  case TRIANGLE_POLY: {
    return poly_triangle(ampl, inc, phase, integrator, dc_x, dc_y, dc_blocker) *
           vol;
  } break;
  case SAW_POLY: {
    return poly_saw(ampl, inc, phase) * vol;
  } break;
  case SAW_RAW: {
    return sawtooth(ampl, phase) * vol;
  } break;
  case TRIANGLE_RAW: {
    return triangle(ampl, phase) * vol;
  } break;
  case PULSE_RAW: {
    return square(ampl, phase, coeff) * vol;
  } break;
  }
}

static void loop_oscilators(struct voice *v, f32 sum[],
                            const struct osc_config *osc_cfg,
                            const struct configs *cfg, f32 dc_blocker,
                            u32 osc_c) {
  const i32 channels = cfg->ivals[CHANNELS_VAL].value;
  const i32 samplerate = cfg->ivals[SAMPLE_RATE_VAL].value;

  for (i32 c = 0; c < channels; c++) {
    sum[c] = 0.0f;
  }

  for (u32 i = 0; i < osc_c; i++) {
    struct osc_state *state = &v->osc[i];

    if (!oscilator_state_on(&v->active, &state->envelope_state)) {
      continue;
    }

    const f32 base_freq = v->base_freq;
    const f32 octave_inc = osc_cfg->spec[OCTAVE_VAL].value;
    const f32 detune = osc_cfg->spec[DETUNE_VAL].value;
    const i32 sample_rate = cfg->ivals[SAMPLE_RATE_VAL].value;
    const f32 ampl = v->amplitude;

    f32 freq = base_freq * octave_inc * detune;
    freq = set_vibrato(freq, state->oscilator_states, cfg);

    const f32 inc = freq / (f32)sample_rate;
    const f32 dt = 1.0f / (f32)sample_rate;

    const i32 waveform_id = osc_cfg->waveform_id;
    for (i32 c = 0; c < channels; c++) {
      state->gen[GEN_ARRAY_RAW][c] =
       generate(ampl, waveform_id, state->oscilator_states, osc_cfg->spec, inc,
                dc_blocker);
    }
    // inc (ie: time interval per sample) /
    // (cutoff_in_seconds + time interval)
    const f32 alpha_high =
     (dt) / (CUTOFF_IN_SEC(HZ_TO_RAD_PER_SEC(NYQUIST((f32)sample_rate))) + dt);
    const f32 alpha_low = (dt) / (CUTOFF_IN_SEC(HZ_TO_RAD_PER_SEC(20.0f)) + dt);
    // Basic interpolation using a cutoff alpha

    f32 *envelope = &state->oscilator_states[ENVELOPE_VAL];
    const f32 *atk = &osc_cfg->env[ATTACK_VAL].value;
    const f32 *dec = &osc_cfg->env[DECAY_VAL].value;
    const f32 *sus = &osc_cfg->env[SUSTAIN_VAL].value;
    const f32 *rel = &osc_cfg->env[RELEASE_VAL].value;

    f32 *raw = state->gen[GEN_ARRAY_RAW];
    f32 *low = state->gen[GEN_ARRAY_LOW];
    f32 *high = state->gen[GEN_ARRAY_HIGH];

    adsr(envelope, &state->envelope_state, atk, dec, sus, rel, samplerate);
    for (i32 c = 0; c < channels; c++) {
      // adsr and compress per osc
      raw[c] *= *envelope;
      // filter
      high[c] += linear_interpolate(raw[c], high[c], alpha_high);
      low[c] += linear_interpolate(raw[c], low[c], alpha_low);
      // mix and sum
      raw[c] = HIGH_MIX * high[c] + LOW_MIX * low[c];
      sum[c] += tanhf((raw[c] / (f32)osc_c) * cfg->fvals[OSC_GAIN_VAL].value);
    }

    state->oscilator_states[PHASE_VAL] += inc;
    if (state->oscilator_states[PHASE_VAL] >= 1.0f) {
      state->oscilator_states[PHASE_VAL] -= 1.0f;
    }
    state->oscilator_states[TIME_VAL] += dt;
  }
}

static void loop_voicings(struct layer *l, f32 wave_samples[CHANNEL_MAX]) {
  for (i32 c = 0; c < CHANNEL_MAX; c++) {
    wave_samples[c] = 0.0;
  }
  const i32 channels = l->pb_cfg.ivals[CHANNELS_VAL].value;
    for (u32 v = 0; v < VOICE_MAX; v++) {
      switch (channels) {
      default:
        break;
      case MONO: {
        f32 sum[channels];
        loop_oscilators(&l->voices[v], sum, &l->osc_cfg, &l->pb_cfg,
                        l->dc_blocker, l->osc_count);
        wave_samples[0] += (sum[0] / VOICE_MAX);
      } break;

      case STEREO: {
        f32 angle = (0.5f * (PI / 2.0f));
        f32 left = cosf(angle);
        f32 right = sinf(angle);

        f32 sum[channels];
        loop_oscilators(&l->voices[v], sum, &l->osc_cfg, &l->pb_cfg,
                        l->dc_blocker, l->osc_count);
        wave_samples[0] += ((sum[0] * left) / VOICE_MAX);
        wave_samples[1] += ((sum[1] * right) / VOICE_MAX);
      } break;
      }
    }
    for (i32 c = 0; c < CHANNEL_MAX; c++) {
      if(wave_samples[c] > 0.0f){
        wave_samples[c] =
        tanhf(wave_samples[c] * l->pb_cfg.fvals[SAMPLE_GAIN_VAL].value);
      }
    }
}

static void loop_samples(size_t count, f32 *samplebuffer, struct layer *l) {
  const size_t channels = (size_t)l->pb_cfg.ivals[CHANNELS_VAL].value;
  for (size_t n = 0; n < count / channels; n++) {
    f32 wave_samples[CHANNEL_MAX];
    loop_voicings(l, wave_samples);
    for (size_t c = 0; c < channels; c++) {
      samplebuffer[n * channels + c] =
       wave_samples[c] * l->pb_cfg.fvals[MAIN_VOLUME_VAL].value;
    }
  }
}

void stream_callback(void *data, SDL_AudioStream *stream, i32 add, i32 total) {
  (void)total;
  struct layer *l = (struct layer *)data;
  if (!l) {
    return;
  }

  size_t sample_count = (u32)add / sizeof(f32);
  while (sample_count > 0) {
    f32 samples[SAMPLE_PER_CALLBACK];
    memset(samples, 0, sizeof(f32) * SAMPLE_PER_CALLBACK);
    const size_t valid_samples = SDL_min(sample_count, SDL_arraysize(samples));
    loop_samples(valid_samples, samples, l);
    if (l->delay_active) {
      loop_delay(valid_samples, samples, &l->pb_cfg, &l->dl);
    }
    stream_feed(stream, samples, (i32)valid_samples * (i32)sizeof(f32));
    render_push(samples, valid_samples, l->layer_window, WINDOW_RESOLUTION);
    sample_count -= valid_samples;
  }
}

SDL_AudioSpec make_audio_spec(i32 channels, i32 samplerate) {
  SDL_AudioSpec spec = {
    SDL_AUDIO_F32,
    channels,
    samplerate,
  };
  return spec;
}

struct playback_device open_audio_device(void) {
  u32 id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
  if (!id) {
    return (struct playback_device){ 0, NULL, { 0, 0, 0 }, false };
  }
  SDL_AudioSpec obtained = { 0 };
  SDL_GetAudioDeviceFormat(id, &obtained, NULL);
  const char *name = SDL_GetAudioDeviceName(id);
  printf("Device name: %s, Output spec: 0x%X - %d - %d\n", name,
         obtained.format, obtained.channels, obtained.freq);
  return (struct playback_device){ id, NULL, obtained, true };
}

bool audio_stream_clear(SDL_AudioStream *stream) {
  if (stream) {
    if (!SDL_ClearAudioStream(stream)) {
      printf("Could not clear queued audio: %s\n", SDL_GetError());
      return false;
    }
    return true;
  }
  return false;
}

// only call from main so it's still easy to free resources
// upon failure
bool audio_steam_ch_input_spec(SDL_AudioStream *stream, u32 dev_id,
                               const SDL_AudioSpec *internal) {
  if (dev_id) {
    if (!pause_audio(dev_id)) {
      return false;
    }

    if (!audio_stream_clear(stream)) {
      return false;
    }

    audio_stream_unbind(stream);
    if (!set_audio_stream_format(stream, internal, NULL)) {
      close_audio_device(dev_id);
      audio_stream_destroy(stream);
      return false;
    }

    if (!audio_stream_bind(stream, dev_id)) {
      return false;
    }

    if (!resume_audio(dev_id)) {
      return false;
    }

    return true;
  }

  return false;
}

bool set_audio_stream_format(SDL_AudioStream *stream,
                             const SDL_AudioSpec *internal,
                             const SDL_AudioSpec *output) {
  if (!stream) {
    return false;
  }

  if (!SDL_SetAudioStreamFormat(stream, internal, output)) {
    printf("Failed to update stream format: %s\n", SDL_GetError());
    return false;
  }
  return true;
}

SDL_AudioStream *audio_stream_create(const SDL_AudioSpec *internal,
                                     const SDL_AudioSpec *output) {
  SDL_AudioStream *stream = SDL_CreateAudioStream(internal, output);
  if (!stream) {
    printf("%s\n", SDL_GetError());
    return NULL;
  }
  if (internal) {
    printf("Created stream with internal spec: (%d %d "
           "%d)\n",
           internal->channels, internal->freq, internal->format);
  }
  return stream;
}

bool set_audio_callback(SDL_AudioStream *stream, struct layer *data) {
  if (!SDL_SetAudioStreamGetCallback(stream, stream_callback, data)) {
    printf("%s\n", SDL_GetError());
    return false;
  }
  return true;
}

void audio_stream_unbind(SDL_AudioStream *stream) {
  if (stream) {
    SDL_UnbindAudioStream(stream);
  }
}

bool audio_stream_bind(SDL_AudioStream *stream, u32 id) {
  if (stream && id) {
    if (!SDL_BindAudioStream(id, stream)) {
      printf("%s\n", SDL_GetError());
      return false;
    }
    return true;
  }
  return false;
}

bool resume_audio(u32 id) {
  if (id) {
    if (!SDL_ResumeAudioDevice(id)) {
      printf("Failed to resume: %s\n", SDL_GetError());
      return false;
    }
    return true;
  }
  return false;
}

bool pause_audio(u32 id) {
  if (id) {
    if (!SDL_PauseAudioDevice(id)) {
      printf("Failed to pause: %s\n", SDL_GetError());
      return false;
    }
    return true;
  }
  return false;
}

void close_audio_device(u32 id) {
  if (id) {
    SDL_CloseAudioDevice(id);
  }
}

void audio_stream_destroy(SDL_AudioStream *stream) {
  if (stream) {
    SDL_DestroyAudioStream(stream);
  }
}