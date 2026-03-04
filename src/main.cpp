#include "audio.hpp"
#include "controller.hpp"
#include "scripts.hpp"

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

  Lua_Container lua;
  Lua_Cfg cfg;
  if(lua.initialize()){
    cfg = lua.load_cfg("lua/config.lua");
  }
  std::cout << cfg.get_state() << std::endl;

  Synth syn;
  Controller controller(name_arg);
  Audio_Sys audio(syn.get_audio_cfg().channels,
                  syn.get_audio_cfg().sample_rate);

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
    controller.read_input(1);
    const i32 *buf = controller.get_msgbuf();

    switch (buf[INPUT_TYPE::STATUS]) {
    default:
      break;
    case NOTE_ON: {
      syn.loop_voicings_on(buf[INPUT_TYPE::MSG_ONE]);
    } break;

    case NOTE_OFF: {
      syn.loop_voicings_off(buf[INPUT_TYPE::MSG_ONE]);
    } break;
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
