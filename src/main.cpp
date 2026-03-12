#include "../inc/audio.hpp"
#include "../inc/controller.hpp"

#include <ctime>
#include <iostream>
#include <portmidi.h>

static bool initialize(void);
static bool sdl_check_quit(void);
static bool quit(void);

static bool initialize(void) {
  if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
    std::cerr << "Failed to initialize SDL! -> " << SDL_GetError() << std::endl;
    return false;
  }

  if (Pm_Initialize() < 0) {
    std::cerr << "Failed to initialize PortMidi!" << std::endl;
    return false;
  }
  return true;
}

static bool sdl_check_quit(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    default:
      return false;

    case SDL_EVENT_QUIT: {
      return true;
    } break;
    }
  }
  return false;
}

static bool quit(void) {
  SDL_Quit();
  if (Pm_Terminate() < 0) {
    std::cerr << "PortMidi failed to terminate correctly!" << std::endl;
    return false;
  }
  return true;
}

int main(int argc, char **argv) {
  const char *name_arg = NULL;
  if (argc > 1 && argc < 3) {
    name_arg = argv[1];
  } else {
    std::cout << "Usage: sgsa device-name" << std::endl;
    return 0;
  }
  srand((unsigned int)time(NULL));

  if (!initialize()) {
    return 0;
  }
  const i32 compiled = SDL_VERSION;
  const i32 linked = SDL_GetVersion();

  std::cout << "Compiled SDL Version: " << SDL_VERSIONNUM_MAJOR(compiled) << "."
            << SDL_VERSIONNUM_MINOR(compiled) << "."
            << SDL_VERSIONNUM_MICRO(compiled) << "." << std::endl;

  std::cout << "Linked SDL Version: " << SDL_VERSIONNUM_MAJOR(linked) << "."
            << SDL_VERSIONNUM_MINOR(linked) << "."
            << SDL_VERSIONNUM_MICRO(linked) << "." << std::endl;

  Synth syn;
  Controller controller(name_arg);
  Audio_Sys audio(syn.get_synth_cfg().channels, syn.get_synth_cfg().sample_rate);

  audio.open(&syn);
  controller.open();

  const u32 FPS = 120;
  const u32 FG = 1000 / FPS;
  bool running = true;
  while (running) {
    const u64 START = SDL_GetTicks();
    if (sdl_check_quit()) {
      running = false;
    }

    controller.clear_msg_buf();
    const i32 event_count = controller.read_input();

    for(i32 i = 0; i < event_count; i++){
      const PmEvent *ev = controller.get_event_at(i);
      if(!ev){
        continue;
      }
      Midi_Input_Msg msg = controller.parse_event(*ev);

      
      switch(msg.status){
        case PITCH_BEND:{
          Modulations& mods = syn.get_mods();
          mods.set_pitch_bend(mods.calculate_pitch_bend(TWO_SEMITONE_CENTS, controller.normalize_pitch_bend(msg.msg2)));
        } break;

        case NOTE_ON: {
          syn.loop_voicings_on(msg.msg1);
        }break;
        case NOTE_OFF: {
          syn.loop_voicings_off(msg.msg1);
        }break;
      }
    }

    const u64 FT = SDL_GetTicks() - START;
    if (FT < FG) {
      const u32 DELAY = (u32)(FG - FT);
      SDL_Delay(DELAY);
    }
  }

  audio.close();
  controller.close();
  quit();
  return 0;
}
