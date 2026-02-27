#include "sgsa.hpp"
#include "util.hpp"
#include <cmath>
#include <iostream>

const f32 PI = 3.141592653589793f;
const i32 CHUNK_MAX = 128;
const f32 VIBRATO_ON = 0.33f;

//d->voices[i].increment_phase(d->voices[i].generative_states[STATE_LFO2], ((120.0f / 60.0f) * 1.0f / (f32)d->samplerate), 1.0f);
//f32 amp = 0.5f * (1.0f + sinf(2.0f * PI * d->voices[i].generative_states[STATE_LFO2]));

static SDL_AudioSpec make_spec(i32 chan, i32 sr);
static bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len);
static void generate_loop(struct Audio_Data *d, size_t count, f32 *sample_buffer);
static f32 generate(const struct Wave_Table *wt, struct Voice *v, f32 freq);
static void voice_loop(struct Audio_Data *d, f32 generated[CHANNEL_MAX]);

static SDL_AudioSpec make_spec(i32 chan, i32 sr) {
    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_F32;
    spec.channels = chan;
    spec.freq = sr;
    return spec;
}
// Safety is my middle name baby (It's not)
static f32 generate(const struct Wave_Table *wt, struct Voice *v, f32 freq){
    const u8 i = wt->index_octave(freq);
    const f32 *wave = wt->tables[wt->current_table][i * 2];
    i32 index = (i32)(v->generative_states[STATE_PHASE]);
    if(index < 0) {
        index += TABLE_SIZE;
    }
    return wave[index % TABLE_SIZE];
}

static void voice_loop(struct Audio_Data *d, f32 generated[CHANNEL_MAX]){
    f32 sums[CHANNEL_MAX] = {0.0f, 0.0f};
    for(i32 i = 0; i < MAX_VOICE; i++){
        if(!is_generating(d->voices[i].voice_state)) {
            continue;
        }

        const f32 vib_inc = d->vibrato_rate / (f32)d->samplerate;        
        d->voices[i].increment_phase(d->voices[i].generative_states[STATE_LFO1], vib_inc, 1.0f);
        
        f32 vib = 0.0f;
        if(d->voices[i].generative_states[STATE_TIME] > VIBRATO_ON){
            vib = d->voices[i].vibrato(d->vibrato_depth);
        }

        const f32 dt = 1.0f / (f32)d->samplerate;
        d->voices[i].increment_time(d->voices[i].generative_states[STATE_TIME], dt);     

        const f32 freq_inc = TABLE_SIZE * (d->voices[i].freq + vib) / (f32)d->samplerate;
        d->voices[i].increment_phase(d->voices[i].generative_states[STATE_PHASE], freq_inc, TABLE_SIZE);
        for(i32 c = 0; c < d->channels; c++){
            d->voices[i].gen[c] = generate(&d->wave_table, &d->voices[i], d->voices[i].freq + vib);
            d->voices[i].adsr(d->samplerate, d->attack, d->decay, d->sustain, d->release);
            d->voices[i].gen[c] *= d->voices[i].generative_states[STATE_ENVELOPE];
            sums[c] += d->voices[i].gen[c];
        }

    }

    for(i32 c = 0; c < d->channels; c++){
        const f32 scale = voice_rms(d->voices, c);
        generated[c] = sums[c] * scale;
    }
}

static void generate_loop(struct Audio_Data *d, size_t count, f32 *sample_buffer){
    for(i32 n = 0; n < (i32)count / d->channels; n++){
        f32 generated[CHANNEL_MAX] = {0.0f, 0.0f};
        voice_loop(d, generated);
        for(i32 c = 0; c < d->channels; c++){
            sample_buffer[n * d->channels + c] = generated[c];
        }
    }
}

static bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len) {
  return SDL_PutAudioStreamData(stream, samples, len);
}

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

Voice::Voice(void) 
: voice_state(0), freq(0.0f), midi_key(0), generative_states(), gen() {
    voice_state = set_bit(0, ENVELOPE_OFF | VOICE_OFF);
    memset(gen, 0, sizeof(f32) * CHANNEL_MAX);
    for(i32 j = 0; j < STATE_END; j++){
        generative_states[j] = 0.0f;
    }
}

void Voice::increment_time(f32& time, f32 inc){
    time += inc;
}


void Voice::increment_phase(f32& phase, f32 inc, f32 max){
    phase += inc;
    if(phase >= max){
        phase -= max;
    }
}

void Wave_Table::print_table(f32 table[TABLE_SIZE]){
    for(i32 i = 0; i < TABLE_SIZE; i++){
        std::cout << "(" << table[i] << ")";
    }
    std::cout << std::endl;
}

u8 Wave_Table::index_octave(f32 freq) const {
    for(u8 i = 0; i < OCTAVES - 1; i++){
        const f32 base = tables[current_table][i * 2 + 1][0];
        const f32 next = tables[current_table][(i + 1) * 2 + 1][0];
        if(freq >= base && freq < next) {
            return i;
        }
    }
    return OCTAVES - 1;
}

Wave_Table::Wave_Table(f32 duty_cycle_coeff, i32 sample_rate) 
: cycle(duty_cycle_coeff), tables(), current_table(0) {
    const i32 N = TABLE_SIZE;
    const f32 C0 = 16.35f;

    for(i32 o = 0; o < OCTAVES; o++){
        const f32 freq = C0 * powf(2.0f, (f32)o);
        const i32 end = (i32)(NYQUIST((f32)sample_rate) / freq);
        tables[TABLE_SAW][o * 2 + 1][0] = freq;

        for(i32 n = 0; n < N; n++){
            const f32 phase = (f32)n / N;
            f32 sum = 0.0f;
            for(i32 k = 1; k <= end; k++){
                f32 sign = powf(-1.0f, (f32)k);
                sum += sign * sinf(2.0f * PI * (f32)k * phase) / (f32)k;
            }
            tables[TABLE_SAW][o * 2][n] = -((2.0f * 1.0f) / PI) * sum;
        }
    }
}

Audio_Data::Audio_Data(i32 chan, i32 sr, f32 atk, f32 dec, f32 sus, f32 rel, f32 cyc, f32 vrate, f32 vdepth) 
: channels(chan), samplerate(sr), attack(atk), decay(dec), sustain(sus), release(rel), 
vibrato_rate(vrate), vibrato_depth(vdepth), voices(), wave_table(cyc, sr)
{
    std::cout << "(Channels: " << channels << ") "
    << "(Sample rate: " << samplerate << ") "
    << "(Attack: " << attack << ") "
    << "(Decay: " << decay << ") "
    << "(Sustain: " << sustain << ") "
    << "(Release: " << release << ") " << std::endl;
}

f32 Voice::vibrato(f32 depth){
    return depth * sinf(2.0f * PI * generative_states[STATE_LFO1]);
}

void Voice::adsr(i32 samplerate, f32 atk, f32 dec, f32 sus, f32 rel){
    switch(voice_state){
        case VOICE_ON | ENVELOPE_ATTACKING | ENVELOPE_ON: {
            generative_states[STATE_ENVELOPE] += ATTACK_INCREMENT((f32)samplerate, atk);
            if(generative_states[STATE_ENVELOPE] >= 1.0f){
                generative_states[STATE_ENVELOPE] = 1.0f;
                voice_state = set_bit(voice_state, VOICE_ON | ENVELOPE_DECAYING | ENVELOPE_ON);
            }
            return;
        } break;

        case VOICE_ON | ENVELOPE_DECAYING | ENVELOPE_ON: {
            generative_states[STATE_ENVELOPE] -= DECAY_INCREMENT((f32)samplerate, dec, sus);
            if (generative_states[STATE_ENVELOPE] <= sus) {
                generative_states[STATE_ENVELOPE] = sus;
                voice_state = set_bit(0, VOICE_ON | ENVELOPE_SUSTAINING | ENVELOPE_ON);
            }       
            return;
        }break;

        case VOICE_OFF | ENVELOPE_RELEASING | ENVELOPE_ON: {
            generative_states[STATE_ENVELOPE] -= RELEASE_INCREMENT((f32)samplerate, rel);
            if (generative_states[STATE_ENVELOPE] <= 0.0f) {
                generative_states[STATE_ENVELOPE] = 0.0f;
                voice_state = set_bit(0, ENVELOPE_OFF | VOICE_OFF);
            }
            return;
        }break;

        case VOICE_ON | ENVELOPE_SUSTAINING | ENVELOPE_ON: {
            return;
        }break;

        case VOICE_OFF | ENVELOPE_OFF: {
            return;
        }break;
    }
}

Audio::Audio(i32 chan, i32 sr, f32 atk, f32 dec, f32 sus, f32 rel, f32 cyc, f32 vrate, f32 vdepth) 
: valid(true), dev(0), stream(NULL), internal(make_spec(chan, sr)), output({SDL_AUDIO_F32, 0, 0}), data(chan, sr, atk, dec, sus, rel, cyc, vrate, vdepth) {
    if(!(open_audio_device() && create_audio_stream())){
        valid = false;
        return;
    }
    
    if(!(set_audio_callback(&data) && bind_stream())){
        valid = false;
        return;
    }
}

void Audio::clear(void){
    if(!SDL_ClearAudioStream(stream)){
        std::cerr << "Failed to clear stream: " << SDL_GetError() << std::endl;
    }
}

void Audio::quit(void){
    pause();
    clear();
    unbind_stream();
    destroy_audio_stream();
    close_audio_device();
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