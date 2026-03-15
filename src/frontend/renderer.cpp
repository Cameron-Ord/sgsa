#include "../../inc/gui.hpp"
#include <iostream>
#include <cmath>

bool Rect::point_in_rect(i32 _x, i32 _y) const {
  return (_x >= x && _x <= x + w) && (_y >= y && _y <= y + h);
}

// She const on my non const renderer data until I constawsdk pnlasdas
void Renderer::render_param_list(const std::array<ParamF32, S_PARAM_COUNT> &items, const Glyphs& g) const {
  const f32 width_f32 = static_cast<f32>(window_width);
  for(size_t i = 0; i < items.size(); i++){
    const ParamF32& p = items[i];

    const i32 rect_width = g.get_string_width(p.name);
    const i32 n = static_cast<i32>(i);
    const i32 y = n * g.get_line_skip();
    const i32 x = static_cast<i32>(roundf((p.value - p.min) / (p.max - p.min) * width_f32));

    Rect rect{x, y, rect_width, g.get_line_skip()};

    render_rect_i(rect.x - (rect_width / 2), rect.y, rect.w, rect.h);
    render_string(g, p.name, y, x - (rect_width / 2));
  }
}

i32 Renderer::render_string(const Glyphs& g, const std::string& str, const i32& y, const i32& start_x) const {
  i32 x = start_x;
  for(size_t i = 0; i < str.size(); i++){
    const u8 c = static_cast<u8>(str[i]);
    const Glyph_Entry *glyph = g.get_glyph_at(c, COLOUR_BASE);
    if(!glyph){
      continue;
    }
    render_char(glyph, y, x);
    x += glyph->width;
  }

  return x - start_x;
}

void Renderer::render_rect_i(i32 x, i32 y, i32 w, i32 h) const {
  SDL_FRect rect{(f32)x, (f32)y, (f32)w, (f32)h};
  SDL_RenderFillRect(r, &rect);
}

void Renderer::render_char(const Glyph_Entry *glyph, const i32& y, const i32& x) const {
  SDL_FRect box = { (f32)x, (f32)y, (f32)glyph->width, (f32)glyph->height };
  if(!SDL_RenderTexture(r, glyph->texture, NULL, &box)){
    std::cerr << "Failed to render character: " << SDL_GetError() << std::endl;
  }
}
