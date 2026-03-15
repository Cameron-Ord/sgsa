#include "../../inc/gui.hpp"

void Events::run_events(std::vector<Event_Command>& commands, Window& _win){
  for(size_t i = 0; i < commands.size(); i++){
    switch(commands[i].type){
      default: break;
      case Event_Command::keydown:{
        switch(commands[i].key.keycode){
          default: break;
          case SDLK_UP:{
            up();
          } break;
          case SDLK_LEFT:{
            emit_dec_param();
          } break;
          case SDLK_RIGHT:{
            emit_inc_param();
          } break;
          case SDLK_DOWN:{
            down();
          } break;
        }
      } break;
      case Event_Command::quit:{
        _win.set_quit(true);
      } break;
    }
  }
}

void Events::emit_inc_param(void){
  requests.push_back(Modify_Request(cursor, Modify_Request::REQ_INC));
}

void Events::emit_dec_param(void){
  requests.push_back(Modify_Request(cursor, Modify_Request::REQ_DEC));
}

void Events::down(void){
  const size_t next = cursor + 1;
  cursor = static_cast<SYNTH_PARAMETER>(next % S_PARAM_COUNT);
}

void Events::up(void){
  const i32 _cursor = static_cast<i32>(cursor);
  const i32 prev = _cursor - 1;
  if(prev >= 0){
    cursor = static_cast<SYNTH_PARAMETER>(cursor - 1);
  }
}

std::vector<Event_Command> Events::read_event(void){
  SDL_Event event;
  std::vector<Event_Command> commands(0);

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    default: break;

    case SDL_EVENT_KEY_DOWN: {
      commands.push_back({Event_Command::keydown, Mouse_Event{}, Key_Event{event.key.key, event.key.mod}});
    } break;

    case SDL_EVENT_QUIT: {
      commands.push_back({Event_Command::quit, Mouse_Event{}, Key_Event{}});
    } break;
    }
  }

  return commands;
}
