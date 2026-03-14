#include "../../inc/gui.hpp"
#include <iostream>

void Renderer::render_float_list(std::vector<Param_Float> params, size_t viewport_index, const Glyphs& g){
  const SDL_Rect *view_rect = get_viewport_at(viewport_index);
  if(!view_rect) {
    return;
  }
  const i32 line_skip = g.get_line_skip();
  i32 y = 0;

  for(size_t i = 0; i < params.size(); i++){
    const std::string& str = params[i].param_name;
    render_string(g, str, y);
    y += line_skip;
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

void Renderer::render_char(const Glyph_Entry *glyph, const i32& y, const i32& x) {
  SDL_FRect box = { (f32)x, (f32)y, (f32)glyph->width, (f32)glyph->height };
  if(!SDL_RenderTexture(r, glyph->texture, NULL, &box)){
    std::cerr << "Failed to render character: " << SDL_GetError() << std::endl;
  }
}
