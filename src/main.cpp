#include "../inc/synth.hpp"
#include "../inc/audio_sys.hpp"
#include "../inc/gui.hpp"

#include <ctime>
#include <iostream>
#include <portmidi.h>

static bool initialize(void);
static bool quit(void);
static void listen_event_emits(Events& events, Synth& syn);

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


 //static void listen_event_emits(Events& events, Synth& syn){
   //std::vector<Modify_Request> reqs = events.get_requests();
   //for(size_t i = 0; i < reqs.size(); i++){
     //const Modify_Request& req = reqs[i];
     //switch(req.method){
       //case Modify_Request::REQ_DEC: {
         //syn.dec_param(req.index);
       //} break;
       //case Modify_Request::REQ_INC: {
         //syn.inc_param(req.index);
       //} break;
     //}
   //}
   //events.clear_requests();
 //}

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


  Glyphs glyphs("arial.ttf", 18.0f);
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

  audio.open(&syn);
  controller.open();
  win.show_window();

  const u32 FPS = 120;
  const u32 FG = 1000 / FPS;
  while (!win.get_quit()) {
    const u64 START = SDL_GetTicks();
  
    win.get_render_class().clear_colour(0, 0, 0, 255);
    win.get_render_class().clear();
    
    std::vector<Event_Command> sdl_cmds = win.get_event_class().read_event();
    std::vector<Keyboard_Command> midi_cmds = syn.read_event(controller);

    syn.run_events(midi_cmds);
    win._run_events(sdl_cmds);
    
    win.get_render_class().clear_colour(255, 255, 255, 255);
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
