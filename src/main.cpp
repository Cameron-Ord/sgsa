#include "../inc/audio.hpp"
#include "../inc/controller.hpp"
#include "../inc/gui.hpp"

#include <ctime>
#include <iostream>
#include <portmidi.h>

static bool initialize(void);
static bool sdl_check_quit(void);
static bool quit(void);

static bool initialize(void) {
  if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
    std::cerr << "Failed to initialize SDL! -> " << SDL_GetError() << std::endl;
    return false;
  }

  if(!TTF_Init()){
    std::cerr << "Failed to initialize SDL_TTF! -> " << SDL_GetError() << std::endl;
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
  TTF_Quit();
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

  Window win(SDL_WINDOW_HIDDEN, 400, 300);
  if(!win.create_window()){
    quit();
    return 1;
  }
  
  if(!win.get_render_class().create_renderer(win.get_window())){
    quit();
    return 1;
  }


  Glyphs glyphs("arial.ttf", 12.0f);
  if(!glyphs.open()){
    quit();
    return 1;
  }
  glyphs.find_line_skip();
  std::cout << "here1" << std::endl;

  if(!glyphs.table_allocate(win.get_render_class())){
    quit();
    return 1;
  }
  std::cout << "here2" << std::endl;

  Synth syn;
  Controller controller(name_arg);
  Audio_Sys audio(syn.get_synth_cfg().channels, syn.get_synth_cfg().sample_rate);

  audio.open(&syn);
  controller.open();
  win.show_window();

  const u32 FPS = 120;
  const u32 FG = 1000 / FPS;
  bool running = true;
  while (running) {
    const u64 START = SDL_GetTicks();
  
    win.get_render_class().clear_colour(0, 0, 0, 255);
    win.get_render_class().clear();

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
        case CONTROL:{
          switch(msg.msg1){
            case CONTROL_MOD_WHEEL:{
              syn.loop_voicings_fmod(controller.normalize_event(msg.msg2), CONTROL_MOD_WHEEL);
            } break;
          }
        }break;

        case PITCH_BEND:{
          syn.loop_voicings_fmod(controller.normalize_event_bipolar(msg.msg2), PITCH_BEND);
        } break;

        case NOTE_ON: {
          syn.loop_voicings_on(msg.msg1, controller.normalize_event(msg.msg2));
        }break;
        case NOTE_OFF: {
          syn.loop_voicings_off(msg.msg1);
        }break;
      }
    }

    win.get_render_class().present();

    const u64 FT = SDL_GetTicks() - START;
    if (FT < FG) {
      const u32 DELAY = (u32)(FG - FT);
      SDL_Delay(DELAY);
    }
  }

  audio.close();
  controller.close();
  glyphs.close();
  quit();
  return 0;
}
