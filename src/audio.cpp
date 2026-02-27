#include "sgsa.hpp"
#include "util.hpp"
#include <cmath>
#include <iostream>

const f32 PI = 3.141592653589793f;
const i32 CHUNK_MAX = 128;
const f32 VIBRATO_ON = 0.33f;

static SDL_AudioSpec make_spec(i32 chan, i32 sr);
static bool stream_feed(SDL_AudioStream *stream, const f32 samples[], i32 len);
static void generate_loop(struct Audio_Data *d, size_t count, f32 *sample_buffer);
static bool generating(const struct Voice& v);
static f32 generate(const struct Wave_Table *wt, struct Voice *v, i32 sample_rate, f32 vrate, f32 vdepth);
static void voice_loop(struct Audio_Data *d, f32 generated[CHANNEL_MAX]);

static SDL_AudioSpec make_spec(i32 chan, i32 sr) {
    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_F32;
    spec.channels = chan;
    spec.freq = sr;
    return spec;
}
// Safety is my middle name baby (It's not)
static f32 generate(const struct Wave_Table *wt, struct Voice *v, i32 sample_rate, f32 vrate, f32 vdepth){
    const f32 *wave = wt->tables[wt->current_table];
    f32 vib = 0.0f;
    if(v->generative_states[STATE_TIME] > VIBRATO_ON){
        vib = v->vibrato(vdepth);
    }

    i32 index = (i32)(v->generative_states[STATE_PHASE] + vib);
    if(index < 0) {
        index += TABLE_SIZE;
    }
    return wave[index % TABLE_SIZE];
}

static bool generating(const struct Voice& v){
    const bool first = check_bit(v.voice_state, VOICE_ON | ENVELOPE_OFF | ENVELOPE_RELEASING, VOICE_ON);
    const bool second = check_bit(v.voice_state, VOICE_OFF | ENVELOPE_RELEASING, VOICE_OFF | ENVELOPE_RELEASING);
    return first || second;
}

static void voice_loop(struct Audio_Data *d, f32 generated[CHANNEL_MAX]){
    f32 sum = 0.0f;
    i32 active_count = 0;
    for(i32 i = 0; i < MAX_VOICE; i++){
        if(!generating(d->voices[i])) {
            continue;
        }

        const f32 inc = TABLE_SIZE * d->voices[i].freq / (f32)d->samplerate;
        const f32 dt = 1.0f / (f32)d->samplerate;
        d->voices[i].generative_states[STATE_TIME] += dt;

        d->voices[i].generative_states[STATE_PHASE] += inc;
        if(d->voices[i].generative_states[STATE_PHASE] >= TABLE_SIZE){
            d->voices[i].generative_states[STATE_PHASE] -= TABLE_SIZE;
        }

        const f32 mod_inc = d->vibrato_rate / (f32)d->samplerate;
        d->voices[i].generative_states[STATE_PHASE_MOD] += mod_inc;
        if(d->voices[i].generative_states[STATE_PHASE_MOD] >= 1.0f){
            d->voices[i].generative_states[STATE_PHASE_MOD] -= 1.0f;
        }

        for(i32 c = 0; c < d->channels; c++){
            d->voices[i].gen[c] = generate(&d->wave_table, &d->voices[i], d->samplerate, d->vibrato_rate, d->vibrato_depth);
            d->voices[i].adsr(d->samplerate, d->attack, d->decay, d->sustain, d->release);
            d->voices[i].gen[c] *= d->voices[i].generative_states[STATE_ENVELOPE];
            sum += d->voices[i].gen[c];
        }

        active_count++;    
    }

    for(i32 c = 0; c < d->channels; c++){
        generated[c] = sum * (1.0f / sqrtf((f32)active_count));
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

void Wave_Table::print_table(f32 table[TABLE_SIZE]){
    for(i32 i = 0; i < TABLE_SIZE; i++){
        std::cout << "(" << table[i] << ")";
    }
    std::cout << std::endl;
}


Wave_Table::Wave_Table(f32 duty_cycle_coeff) 
: cycle(duty_cycle_coeff), tables(), current_table(0) {
    const i32 N = TABLE_SIZE;
    const i32 HALFN = N / 2;

    for(i32 i = 0, j = HALFN; i < HALFN && j < N; i++, j++){
        tables[TABLE_TRIANGLE][i] = 2.0f * (f32)i / (HALFN - 1) - 1.0f;
        tables[TABLE_TRIANGLE][j] = 2.0f * ((N - 1) - (f32)j) / (HALFN - 1) - 1.0f;
    }
 
    for(i32 i = 0, j = HALFN; i < HALFN && j < N; i++, j++){
        tables[TABLE_SQUARE][i] = 1.0f;
        tables[TABLE_SQUARE][j] = -1.0f;
    }
 
    const i32 cycN = (i32)floorf((N * cycle));
    for(i32 i = 0; i < cycN; i++){
        tables[TABLE_PULSE][i] = 1.0f;
    }
    for(i32 i = cycN; i < TABLE_SIZE; i++){
        tables[TABLE_PULSE][i] = -1.0f;
    }

    std::cout << "==TRIANGLE==" << std::endl;
    print_table(tables[TABLE_TRIANGLE]);
    std::cout << "==PULSE==" << std::endl;
    print_table(tables[TABLE_PULSE]);
    std::cout << "==SQUARE==" << std::endl;
    print_table(tables[TABLE_SQUARE]);
}

Audio_Data::Audio_Data(i32 chan, i32 sr, f32 atk, f32 dec, f32 sus, f32 rel, f32 cyc, f32 vrate, f32 vdepth) 
: channels(chan), samplerate(sr), attack(atk), decay(dec), sustain(sus), release(rel), vibrato_rate(vrate), vibrato_depth(vdepth), voices(), wave_table(cyc) {
    std::cout << "(Channels: " << channels << ") "
    << "(Sample rate: " << samplerate << ") "
    << "(Attack: " << attack << ") "
    << "(Decay: " << decay << ") "
    << "(Sustain: " << sustain << ") "
    << "(Release: " << release << ") " << std::endl;
}

f32 Voice::vibrato(f32 depth){
    return depth * sinf(2.0f * PI * generative_states[STATE_PHASE_MOD]);
}

void Voice::adsr(i32 samplerate, f32 atk, f32 dec, f32 sus, f32 rel){
    switch(voice_state){
        case VOICE_ON | ENVELOPE_ATTACKING: {
            generative_states[STATE_ENVELOPE] += ATTACK_INCREMENT((f32)samplerate, atk);
            if(generative_states[STATE_ENVELOPE] >= 1.0f){
                generative_states[STATE_ENVELOPE] = 1.0f;
                voice_state = clear_bit(voice_state, ENVELOPE_ATTACKING);
                voice_state = set_bit(voice_state, ENVELOPE_DECAYING);
            }
            return;
        } break;

        case VOICE_ON | ENVELOPE_DECAYING: {
            generative_states[STATE_ENVELOPE] -= DECAY_INCREMENT((f32)samplerate, dec, sus);
            if (generative_states[STATE_ENVELOPE] <= sus) {
                generative_states[STATE_ENVELOPE] = sus;
                voice_state = clear_bit(voice_state, ENVELOPE_DECAYING);
                voice_state = set_bit(voice_state, ENVELOPE_SUSTAINING);
            }       
            return;
        }break;

        case VOICE_OFF | ENVELOPE_RELEASING: {
            generative_states[STATE_ENVELOPE] -= RELEASE_INCREMENT(generative_states[STATE_ENVELOPE], (f32)samplerate, rel);
            if (generative_states[STATE_ENVELOPE] <= 0.0f) {
                generative_states[STATE_ENVELOPE] = 0.0f;
                voice_state = clear_bit(voice_state, ENVELOPE_RELEASING);
                voice_state = set_bit(voice_state, ENVELOPE_OFF);
            }
            return;
        }break;

        case VOICE_ON | ENVELOPE_SUSTAINING: {
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