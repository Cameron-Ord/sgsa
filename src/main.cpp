#include "sgsa.hpp"
#include "util.hpp"
#include <iostream>
#include <portmidi.h>

const i32 SAMPLE_RATE = 13379;
const i32 CHANNELS = 1;

static bool initialize(void);
static bool sdl_check_quit(void);

static bool sdl_check_quit(void){
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      default: return false;
       
      case SDL_EVENT_QUIT: {
        return true;
      } break;
      }
    }
    return false;
}

int main(int argc, char **argv){
    const char *name_arg = NULL;
    if(argc > 1 && argc < 3){
        name_arg = argv[1];
    } else {
        std::cout << "Usage: sgsa device-name" << std::endl;
        return 0;
    }

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

    Manager manager(CHANNELS, SAMPLE_RATE, name_arg);

    
    const u32 FPS = 120;
    const u32 FG = 1000 / FPS;
    bool running = true;
    while(running){
        const u64 START = SDL_GetTicks();
        if(sdl_check_quit()){
            running = false;
        }

        manager.get_controller().clear_msg_buf();
        manager.get_controller().read_input(1);
        const i32 *buf = manager.get_controller().get_msgbuf();

        switch(buf[INPUT_MSG_STATUS]){
            default: break;
            case NOTE_ON:{
                std::cout << "ON: " << midi_to_freq(buf[INPUT_MSG_ONE]) << std::endl;
            }break;

            case NOTE_OFF:{
                std::cout << "OFF: " << midi_to_freq(buf[INPUT_MSG_ONE]) << std::endl;
            }break;
        }

        const u64 FT = SDL_GetTicks() - START;
        if (FT < FG) {
            const u32 DELAY = (u32)(FG - FT);
            SDL_Delay(DELAY);
        }

    }

    return 0;
}

Manager::Manager(i32 channels, i32 samplerate, const char *name_arg) 
: audio(channels, samplerate), key_events(), controller(name_arg) {
    
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
