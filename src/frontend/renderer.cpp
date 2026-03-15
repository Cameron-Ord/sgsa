#include "../../inc/gui.hpp"
#include <iostream>
#include <cmath>

bool Rect::point_in_rect(i32 _x, i32 _y) const {
  return (_x >= x && _x <= x + w) && (_y >= y && _y <= y + h);
}

void Renderer::render_generic_data(const Glyphs& g){
  std::vector<Generic_Item> *items = get_generic_data_at(data_index);
  if(!items) {
    return;
  }

  const i32 padding = 4;
  const i32 center = (window_height / 2) - (window_height / 4);
  const f32 wwidthf = static_cast<f32>(window_width);

  for(size_t i = 0; i < items->size(); i++){
    const f32 *min = (*items)[i].data.min;
    const f32 *max = (*items)[i].data.max;
    const f32 *value = (*items)[i].data.value;
    if(!min || !max || !value){
      return;
    }

    Rect& rect = (*items)[i].rect;
    const f32 val_scr_posf = ((*value - *min) / (*max - *min)) * wwidthf;
    const i32 val_scr_posi = static_cast<i32>(roundf(val_scr_posf));

    const std::string& str = (*items)[i].data.param_name;
    const i32 str_width = g.get_string_width(str);
    const i32 n = static_cast<i32>(i);
    const i32 y = center + n * (g.get_line_skip() + padding); 
   
    rect.y = y;
    rect.x = val_scr_posi;
    rect.w = str_width + padding;
    rect.h = g.get_line_skip();
 
    render_rect_i(rect.x, rect.y, rect.w, rect.h);
    render_string(g, str, y, val_scr_posi);
  }
}

i32 Renderer::render_string(const Glyphs& g, const std::string& str, const i32& y, const i32& start_x){
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
