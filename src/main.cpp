#include "../inc/synth.hpp"
#include "../inc/audio_sys.hpp"
#include "../inc/gui.hpp"

#include <ctime>
#include <iostream>
#include <portmidi.h>

static bool initialize(void);
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

  if(!glyphs.table_allocate(win.get_render_class())){
    quit();
    return 1;
  }

  Synth syn;
  Controller controller(name_arg);
  Audio_Sys audio(syn.get_channels(), syn.get_sample_rate());

  std::vector<Param_Float> items = {
    Param_Float{ "Attack", syn.get_attack_max(), syn.get_attack_min(), syn.get_attack() },
    Param_Float{ "Decay", syn.get_decay_max(), syn.get_decay_min(), syn.get_decay() },
    Param_Float{ "Sustain", syn.get_sustain_max(), syn.get_sustain_min(), syn.get_sustain() },
    Param_Float{ "Release", syn.get_release_max(), syn.get_release_min(), syn.get_release() },
    Param_Float{ "Low-Pass Filter Hz", syn.get_low_pass_max(), syn.get_low_pass_min(), syn.get_low_pass() },
    Param_Float{ "Gain", syn.get_gain_max(), syn.get_gain_min(), syn.get_gain() },
    Param_Float{ "Tremolo Depth", syn.get_trem_depth_max(), syn.get_trem_depth_min(), syn.get_trem_depth() }
  };
  win.get_render_class().param_list_set_positions(items, glyphs.get_line_skip());

  audio.open(&syn);
  controller.open();
  win.show_window();

  const u32 FPS = 120;
  const u32 FG = 1000 / FPS;
  while (!win.get_quit()) {
    const u64 START = SDL_GetTicks();
  
    win.get_render_class().clear_colour(0, 0, 0, 255);
    win.get_render_class().clear();
    
    std::vector<Event_Command> sdl_cmds = win.read_event();
    std::vector<Keyboard_Command> midi_cmds = syn.read_event(controller);

    syn.run_events(midi_cmds);
    win.run_events(sdl_cmds);
    
    win.get_render_class().clear_colour(255, 255, 255, 255);
    win.get_render_class().render_float_list(items, SCREEN_LEFT, glyphs);
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
