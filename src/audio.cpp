#include "sgsa.hpp"
#include "util.hpp"
#include <iostream>

const i32 SAMPLES_PER_TICK = 128;

static SDL_AudioSpec make_spec(i32 chan, i32 sr);

static SDL_AudioSpec make_spec(i32 chan, i32 sr) {
    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_F32;
    spec.channels = chan;
    spec.freq = sr;
    return spec;
}

void stream_get(void *data, SDL_AudioStream *stream, i32 add, i32 total){
    return;
}

Voice::Voice(void){
    voice_state = set_bit(0, ENVELOPE_OFF | VOICE_OFF);
    gen = 0.0f;
    for(i32 j = 0; j < STATE_END; j++){
        generative_states[j] = 0.0f;
    }
}

Audio_Data::Audio_Data(i32 chan, i32 sr, f32 atk, f32 dec, f32 sus, f32 rel, f32 cyc) 
: channels(chan), samplerate(sr), attack(atk), decay(dec), sustain(sus), release(rel), cycle(cyc) {
    std::cout << "(Channels: " << channels << ") "
    << "(Sample rate: " << samplerate << ") "
    << "(Attack: " << attack << ") "
    << "(Decay: " << decay << ") "
    << "(Sustain: " << sustain << ") "
    << "(Release: " << release << ") "
    << "(Cycle: " << cycle << ") " << std::endl;
}

Audio::Audio(i32 chan, i32 sr, f32 atk, f32 dec, f32 sus, f32 rel, f32 cyc) 
: valid(true), dev(0), stream(NULL), internal(make_spec(chan, sr)), output({SDL_AUDIO_F32, 0, 0}), data(chan, sr, atk, dec, sus, rel, cyc) {
    if(!(open_audio_device() && create_audio_stream())){
        valid = false;
        return;
    }
    
    if(!(set_audio_callback(NULL) && bind_stream())){
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