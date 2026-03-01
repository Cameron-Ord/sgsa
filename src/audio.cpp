#include "sgsa.hpp"
#include "util.hpp"
#include <cmath>
#include <iostream>

static bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len);
static void generate_loop(struct Audio_Data *d, size_t count, f32 *sample_buffer);
static f32 generate(const struct Wave_Table *wt, f32 phase, size_t table_id, f32 freq);
static void voice_loop(struct Audio_Data *d, f32 generated[CHANNEL_MAX]);

const i32 CHUNK_MAX = 128;

void stream_get(void *data, SDL_AudioStream *stream, i32 add, i32 total){
    struct Audio_Data *d = static_cast<struct Audio_Data *>(data);
    if(!d) return;
    // Additional is consumed immediately
    (void)total;
    size_t sample_count = (u32)add / sizeof(f32);
    while(sample_count > 0){
        f32 samples[CHUNK_MAX];
        memset(samples, 0, sizeof(f32) * CHUNK_MAX);
        const size_t actual_samples = SDL_min(sample_count, SDL_arraysize(samples));
        generate_loop(d, actual_samples, samples);
        stream_feed(stream, samples, (i32)actual_samples * (i32)sizeof(f32));
        sample_count -= actual_samples;
    }
    return;
}

// Safety is my middle name baby (It's not)
static f32 generate(const struct Wave_Table *wt, f32 phase, size_t table_id, f32 freq){
    const u8 i = wt->index_octave(freq);
    const f32 *wave = wt->tables[table_id][i];
    size_t j = (size_t)floorf(phase) % wt->size;
    size_t k = (j + 1) % wt->size;
    f32 f = phase - (f32)j;
    //Source: BasicSynth
    return wave[j] + ((wave[k] - wave[j]) * f);
}

static void voice_loop(struct Audio_Data *d, f32 generated[CHANNEL_MAX]){
    f32 sums[CHANNEL_MAX] = {0.0f, 0.0f};
    f32 sums_squared[CHANNEL_MAX] = {0.0f, 0.0f};

    const Audio_Params *ap = &d->ap_;
    size_t count = 0;

    for(size_t i = 0; i < ap->voicings; i++){
        struct Voice *v = &d->voices[i];
        for(size_t o = 0; o < v->osc_count; o++){
          struct Oscilator *osc = &v->oscs[o];
          if(!is_generating(v->voice_state | osc->env_state)) {
            continue;
          }
          const struct Oscilator_Cfg *cfg = &v->cfgs[o];

          const size_t wt_size = d->wave_table.size;
          const f32 freq = v->freq * cfg->detune * cfg->volume * cfg->step;
          const f32 dt = 1.0f / (f32)ap->sample_rate;
          const f32 inc = (f32)wt_size * freq / (f32)ap->sample_rate;
          
          osc->increment_time(dt);     
          osc->increment_phase(inc, (f32)wt_size);

          const Env_Params *ep = &d->envp_;
          for(i32 c = 0; c < ap->channels; c++){
            osc->adsr(v->active_oscilators, ap->sample_rate, ep->attack, ep->decay, ep->sustain, ep->release);
            osc->samples.unfiltered[c] = generate(&d->wave_table, osc->gen_states[STATE_PHASE], cfg->table_id, freq);
            osc->samples.unfiltered[c] *= osc->gen_states[STATE_ENVELOPE];
            osc->samples.lerp(ap->lpf_alpha_low, ap->lpf_alpha_high, c);
            osc->samples.filtered[c] = 0.7f * osc->samples.high[c] + 0.3f * osc->samples.low[c];

            sums[c] += osc->samples.filtered[c];
            sums_squared[c] += osc->samples.filtered[c] * osc->samples.filtered[c];
          }
          count++;
        }
    }
    if(count > 0){
      for(i32 c = 0; c < ap->channels; c++){
          const f32 scale = sqrtf(sums_squared[c] / (f32)count);
          generated[c] = sums[c] * scale;
      }
    }
}

static void generate_loop(struct Audio_Data *d, size_t count, f32 *sample_buffer){
    for(i32 n = 0; n < (i32)count / d->ap_.channels; n++){
        f32 generated[CHANNEL_MAX] = {0.0f, 0.0f};
        voice_loop(d, generated);
        for(i32 c = 0; c < d->ap_.channels; c++){
            sample_buffer[n * d->ap_.channels + c] = generated[c];
        }
    }
}

static bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len) {
  return SDL_PutAudioStreamData(stream, samples, len);
}

Audio::Audio(const Params p, std::vector<Oscilator_Cfg> templates) 
: parameters(p), dev(0), stream(NULL), 
  internal({SDL_AUDIO_F32, p.ap.channels, p.ap.sample_rate}), 
  output({SDL_AUDIO_F32, 0, 0}),  data(parameters, templates, templates.size()) 
{}

bool Audio::open(void){
    if(!(open_audio_device() && create_audio_stream())){
        return false;
    }
    
    if(!(set_audio_callback(&data) && bind_stream())){
        return false;
    }
    return true;
}

void Audio::close(void){
    pause();
    clear();
    unbind_stream();
    destroy_audio_stream();
    close_audio_device();
}

void Audio::clear(void){
    if(!SDL_ClearAudioStream(stream)){
        std::cerr << "Failed to clear stream: " << SDL_GetError() << std::endl;
    }
}


bool Audio::set_audio_callback(void *userdata){ 
    if(!SDL_SetAudioStreamGetCallback(stream, stream_get, userdata)){
        std::cerr << "Failed to set callback: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool Audio::bind_stream(void){
    if(!(stream && dev)) { 
        std::cerr << "Invalid parameters" << std::endl;
        return false; 
    }
    
    if(!SDL_BindAudioStream(dev, stream)){
        std::cerr << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

bool Audio::unbind_stream(void){
    if(!stream){
        std::cerr << "Invalid parameters" << std::endl;
        return false; 
    }

    SDL_UnbindAudioStream(stream);
    return true;
}

bool Audio::open_audio_device(void){
    dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if(!dev){
        std::cerr << SDL_GetError() << std::endl;
        return false;
    }
    SDL_GetAudioDeviceFormat(dev, &output, NULL);
    const char *name = SDL_GetAudioDeviceName(dev);
    std::cout << "Device name: " << name << std::endl;
    std::cout << "Channels: " << output.channels << " Samplerate: " << output.freq << std::endl; 
    return true;
}

bool Audio::create_audio_stream(void){
    stream = SDL_CreateAudioStream(&internal, &output);
    if(!stream){
        std::cerr << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool Audio::close_audio_device(void){
    if(!dev){
        std::cerr << "Invalid parameter" << std::endl;
        return false;
    }
    SDL_CloseAudioDevice(dev);
    return true;
}

bool Audio::destroy_audio_stream(void){
    if(!stream){
        std::cerr << "Invalid parameter" << std::endl;
        return false;
    }
    return true;
}

bool Audio::resume(void){
    if(!dev){
        std::cerr << "Invalid parameter" << std::endl;
        return false;
    }

    if(!SDL_PauseAudioDevice(dev)){
        std::cerr << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

bool Audio::pause(void){
    if(!dev){
        std::cerr << "Invalid parameter" << std::endl;
        return false;
    }

    if(!SDL_ResumeAudioDevice(dev)){
        std::cerr << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}
