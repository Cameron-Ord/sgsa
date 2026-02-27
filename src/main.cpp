#include "sgsa.hpp"
#include "util.hpp"
#include <iostream>
#include <portmidi.h>

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

    const i32 SAMPLE_RATE = 13379;
    const i32 CHANNELS = 2;

    const f32 ATK = 0.0f;
    const f32 DEC = 0.045f;
    const f32 SUS = 0.4f;
    const f32 REL = 0.03f;

    const f32 VRATE = 4.0f;
    const f32 VDEPTH = 2.0f;

    const f32 CYCLE = 0.25f;
    Manager manager(CHANNELS, SAMPLE_RATE, ATK, DEC, SUS, REL, CYCLE, VRATE, VDEPTH, name_arg);
    
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
                manager.get_controller().iterate_input_on(manager.get_audio().get_data(), buf[INPUT_MSG_ONE]);
            }break;

            case NOTE_OFF:{
                manager.get_controller().iterate_input_off(manager.get_audio().get_data(), buf[INPUT_MSG_ONE]);
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

Manager::Manager(i32 chan, i32 sr, f32 atk, f32 dec, f32 sus, f32 rel, f32 cyc, f32 vrate, f32 vdepth, const char *name_arg) 
: audio(chan, sr, atk, dec, sus, rel, cyc, vrate, vdepth), key_events(), controller(name_arg) {
    
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
