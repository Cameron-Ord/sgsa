#include "../../inc/gui.hpp"
#include <iostream>

Window::Window(size_t _window_flags, i32 _width, i32 _height) 
  : flags(_window_flags), w(nullptr), width(_width), height(_height),
  rend(width, height) {}

Window::~Window(void){
  destroy_window();
}

void Window::run_events(std::vector<Event_Command>& commands){
  for(size_t i = 0; i < commands.size(); i++){
    switch(commands[i].type){
      default: break;
      case Event_Command::mouse_down:{
         
      } break;
      case Event_Command::quit:{
        set_quit(true);
      } break;
    }
  }
}

std::vector<Event_Command> Window::read_event(void){
  SDL_Event event;
  std::vector<Event_Command> commands(0);

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    default: break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      commands.push_back({Event_Command::mouse_down, Mouse_Event{ (i32)event.button.x, (i32)event.button.y }, Key_Event{}});
    } break;

    case SDL_EVENT_QUIT: {
      commands.push_back({Event_Command::quit, Mouse_Event{}, Key_Event{}});
    } break;
    }
  }

  return commands;
}

void Window::destroy_window(void){
  if(w){
    SDL_DestroyWindow(w);
  }
}

void Window::hide_window(void){
  if(!SDL_HideWindow(w)){
    std::cerr << "Failed to hide window: " << SDL_GetError() << std::endl;
  }
}

void Window::show_window(void){
  if(!SDL_ShowWindow(w)){
    std::cerr << "Failed to show window: " << SDL_GetError() << std::endl;
  }
}

bool Window::create_window(void){
  SDL_Window *tmp = SDL_CreateWindow("SGSA", width, height, flags);
  if(!tmp){
    std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
    return false;
  }
  w = tmp;
  return true;
}

void Window::update_size(void){
  i32 tmp_width = width, tmp_height = height;
  if(!SDL_GetWindowSize(w, &tmp_width, &tmp_height)){
    std::cerr << "Error querying new window size: " << SDL_GetError() << std::endl;
    return;
  }
  width = tmp_width, height = tmp_height;
}

Renderer::Renderer(const i32& _window_width, const i32& _window_height) 
  : viewports(), r(nullptr), window_width(_window_width), window_height(_window_height) {
  const i32 half_width = window_width / 2;

  SDL_Rect& left = viewports[SCREEN_LEFT];
  SDL_Rect& right = viewports[SCREEN_RIGHT];

  left = SDL_Rect{0, 0, half_width, window_height};
  right = SDL_Rect{half_width, 0, half_width, window_height}; 
}

Renderer::~Renderer(void){
  destroy_renderer();
}

void Renderer::destroy_renderer(void){
  if(r){
    SDL_DestroyRenderer(r);
  }
}

bool Renderer::create_renderer(SDL_Window *w){
  SDL_Renderer *tmp = SDL_CreateRenderer(w, NULL);
  if(!tmp){
    std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
    return false;
  }
  r = tmp;
  return true;
}

SDL_Texture *Renderer::create_texture(SDL_Surface *surf){
  SDL_Texture *texture = SDL_CreateTextureFromSurface(r, surf);
  if(!texture){
    std::cerr << "Error creating texture: " << SDL_GetError() << std::endl;
    return nullptr;
  }
  return texture;
}

void Renderer::set_viewport(const SDL_Rect& viewport){
  SDL_SetRenderViewport(r, &viewport);
}

void Renderer::generic_list_copy(std::vector<Generic_Param>&& list){
  generic_list = std::move(list);
}

const SDL_Rect *Renderer::get_viewport_at(size_t pos) const {
  if(pos < viewports.size()){
    return &viewports[pos];
  }
  return nullptr;
}

void Renderer::clear_colour(u8 r8, u8 g8, u8 b8, u8 a8) {
  SDL_SetRenderDrawColor(r, r8, g8, b8, a8);
}

void Renderer::clear(void) {
  SDL_RenderClear(r);
}

void Renderer::present(void){
  SDL_RenderPresent(r);
}

