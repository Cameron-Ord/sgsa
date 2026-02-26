#include "sgsa.hpp"

#include <iostream>
#include <portmidi.h>

const i32 SAMPLE_RATE = 13379;
const i32 CHANNELS = 1;

static bool initialize(void);

int main(int argc, char **argv){
    if(!initialize()){
        return 0;
    }
    const i32 compiled = SDL_VERSION;
    const i32 linked = SDL_GetVersion();

    std::cout << "Compiled SDL Version: " 
    << SDL_VERSIONNUM_MAJOR(compiled) << "." 
    << SDL_VERSIONNUM_MINOR(compiled) << "."
    << SDL_VERSIONNUM_MICRO(compiled) << "." << std::endl;

    std::cout << "Linked SDL Version: " 
    << SDL_VERSIONNUM_MAJOR(linked) << "." 
    << SDL_VERSIONNUM_MINOR(linked) << "."
    << SDL_VERSIONNUM_MICRO(linked) << "." << std::endl;

    Manager manager(CHANNELS, SAMPLE_RATE);


    return 0;
}

Manager::Manager(i32 channels, i32 samplerate) : audio(channels, samplerate), key_events(), controller() {
    
}

Manager::~Manager(void){
    audio.quit();
    quit();
}

bool Manager::quit(void){
    SDL_Quit();
    if(Pm_Terminate() < 0) {
        std::cerr << "PortMidi failed to terminate correctly!" << std::endl;
        return false;
    }
    return true;
}

static bool initialize(void){
    if(!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS)){
        std::cerr << "Failed to initialize SDL! -> " << SDL_GetError() << std::endl;
        return false;
    }

    if(Pm_Initialize() < 0){
        std::cerr << "Failed to initialize PortMidi!" << std::endl;
        return false;
    }
    return true;
}
