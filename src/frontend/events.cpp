#include "../../inc/gui.hpp"
#include <iostream>

void Events::run_events(std::vector<Event_Command>& commands, Renderer& _rend, Window& _win){
  for(size_t i = 0; i < commands.size(); i++){
    switch(commands[i].type){
      default: break;


      case Event_Command::quit:{
        _win.set_quit(true);
      } break;
    }
  }
}

std::vector<Event_Command> Events::read_event(void){
  SDL_Event event;
  std::vector<Event_Command> commands(0);

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    default: break;

    case SDL_EVENT_QUIT: {
      commands.push_back({Event_Command::quit, Mouse_Event{}, Key_Event{}});
    } break;
    }
  }

  return commands;
}
