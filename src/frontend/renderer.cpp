#include "../../inc/gui.hpp"
#include <iostream>

void Renderer::param_list_set_positions(std::vector<Param_Float>& params, i32 line_skip){
  for(size_t i = 0; i < params.size(); i++){
    const i32 y = static_cast<i32>(i) * (line_skip + params[i].box_h);
    const f32 box_x = (params[i].value - params[i].min) / params[i].max; 
    params[i].string_y = y;
    params[i].box_y = y + line_skip;
    params[i].box_x = static_cast<i32>(box_x * ((static_cast<f32>(window_width) / 2.0f)));
  }
}

void Renderer::render_float_list(const std::vector<Param_Float>& params, size_t viewport_index, const Glyphs& g){
  const SDL_Rect *view_rect = get_viewport_at(viewport_index);
  if(!view_rect) {
    return;
  }

  for(size_t i = 0; i < params.size(); i++){
    const std::string& str = params[i].param_name;
    render_string(g, str, params[i].string_y);
    render_rect_i(params[i].box_x, params[i].box_y, params[i].box_w, params[i].box_h);
  }
}

void Renderer::render_string(const Glyphs& g, const std::string& str, const i32& y){
  i32 x = 0;
  for(size_t i = 0; i < str.size(); i++){
    const u8 c = static_cast<u8>(str[i]);
    const Glyph_Entry *glyph = g.get_glyph_at(c, COLOUR_BASE);
    if(!glyph){
      continue;
    }
    render_char(glyph, y, x);
    x += glyph->width;
  }
}

void Renderer::render_rect_i(i32 x, i32 y, i32 w, i32 h){
  SDL_FRect rect{(f32)x, (f32)y, (f32)w, (f32)h};
  SDL_RenderFillRect(r, &rect);
}

void Renderer::render_char(const Glyph_Entry *glyph, const i32& y, const i32& x) {
  SDL_FRect box = { (f32)x, (f32)y, (f32)glyph->width, (f32)glyph->height };
  if(!SDL_RenderTexture(r, glyph->texture, NULL, &box)){
    std::cerr << "Failed to render character: " << SDL_GetError() << std::endl;
  }
}
