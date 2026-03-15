#include "../../inc/gui.hpp"
#include "../../inc/synth.hpp"

#include <iostream>

Window::Window(size_t _window_flags, i32 _width, i32 _height) 
  : flags(_window_flags), w(nullptr), width(_width), height(_height),
  rend(width, height), events() {}

Window::~Window(void){
  destroy_window();
}

void Window::_run_events(std::vector<Event_Command>& commands){
  events.run_events(commands, rend, *this);
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
  :data_index(ENVELOPE_DATA), render_data(), r(nullptr), 
  window_width(_window_width), window_height(_window_height) {}

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


void Renderer::data_index_inc(size_t val){
  const size_t next = val + 1;
  data_index = (next % GENERIC_DATA_SIZE);
}

void Renderer::data_index_dec(size_t val){
  const i32 prev = static_cast<i32>(val - 1);
  if(prev >= 0){
    data_index = static_cast<size_t>(prev);
  }
}

std::vector<Generic_Item> *Renderer::get_generic_data_at(size_t pos){
  if(pos < render_data.size()){
    return &render_data[pos];
  }
  return nullptr;
}
// This is kinda bad but im not sure how else to communicate the data in such a way that I can mutate it via the frontend
// Maybe i'll figure it out but for now this works
void Renderer::make_render_data(Synth *syn){
  std::vector<Generic_Param> envelope_data{
    Generic_Param{"Attack", &syn->get_attack_max(), &syn->get_attack_min(), &syn->get_attack()},
    Generic_Param{"Decay", &syn->get_decay_max(), &syn->get_decay_min(), &syn->get_decay()},
    Generic_Param{"Sustain", &syn->get_sustain_max(), &syn->get_sustain_min(), &syn->get_sustain()},
    Generic_Param{"Release", &syn->get_release_max(), &syn->get_release_min(), &syn->get_release()}
  };

  std::vector<Generic_Item>& envelope_items = render_data[ENVELOPE_DATA];
  if(envelope_items.size() != envelope_data.size()){
    envelope_items.resize(envelope_data.size());
  }

  for(size_t i = 0; i < envelope_items.size(); i++){
    envelope_items[i].data = envelope_data[i];
  }

  std::vector<Generic_Param> shaping_data{
    Generic_Param{"Volume", &syn->get_volume_max(), &syn->get_volume_min(), &syn->get_volume()},
    Generic_Param{"Gain", &syn->get_gain_max(), &syn->get_gain_min(), &syn->get_gain()},
    Generic_Param{"Low-Pass Filter", &syn->get_low_pass_max(), &syn->get_low_pass_min(), &syn->get_low_pass()},
    Generic_Param{"Tremolo Depth", &syn->get_trem_depth_max(), &syn->get_trem_depth_min(), &syn->get_trem_depth()},
  };
  std::vector<Generic_Item>& shaping_items = render_data[SHAPING_DATA];
  if(shaping_items.size() != shaping_data.size()){
    shaping_items.resize(shaping_data.size());
  }

  for(size_t i = 0; i < shaping_items.size(); i++){
    shaping_items[i].data = shaping_data[i];
  }
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

