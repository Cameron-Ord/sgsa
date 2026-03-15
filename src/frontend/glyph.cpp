#include "../../inc/gui.hpp"
#include <iostream> 

const u8 NULLCHAR = '\0';

Glyph_Entry::Glyph_Entry(void) 
  : texture(), width(0), height(0), c(0) {}

Glyphs::Glyphs(std::string _file_path, f32 _font_size) 
  : glyph_table(), file_path(_file_path), font_size(_font_size),
  font(nullptr), line_skip(0) {

    std::array default_colours{
      SDL_Color{ 0, 0, 0, 255 },
      SDL_Color{ 255, 255, 255, 255 }
    };

    for(size_t i = 0; i < colours.size(); i++){
      colours[i] = default_colours[i];
    }
}

Glyphs::~Glyphs(void){
  table_deallocate();
}

i32 Glyphs::get_string_width(const std::string& str) const {
  i32 x = 0;
  for(size_t i = 0; i < str.size(); i++){
    const u8 c = static_cast<u8>(str[i]);
    const Glyph_Entry *glyph = get_glyph_at(c, COLOUR_BASE);
    if(!glyph){
      continue;
    }
    x += glyph->width;
  }
  return x;
}

const Glyph_Entry *Glyphs::get_glyph_at(const u8 ch, size_t colour_index) const {
  if(colour_index >= glyph_table.size() && glyph_table.size() > 0){
    colour_index = glyph_table.size() - 1;
  } else if(glyph_table.size() == 0) {
    return nullptr;
  }

  if(ch >= 32 && ch < ASCII_SIZE){
    return &glyph_table[colour_index][ch];
  } else {
    return &glyph_table[colour_index]['?'];
  }
}

bool Glyphs::open(void){
  TTF_Font *tmp = TTF_OpenFont(file_path.c_str(), font_size);
  if(!tmp){
    std::cerr << "Failed to open font from path: " << file_path << std::endl;
    std::cerr << "TTF Error: " << SDL_GetError() << std::endl;
    return false;
  }
  font = tmp;
  return true;
}

void Glyphs::close(void){
  if(font) {
    TTF_CloseFont(font);
  }
}

void Glyphs::find_line_skip(void){
  if(font){
    line_skip = TTF_GetFontLineSkip(font);
  }
}

SDL_Surface *Glyphs::create_glyph_surface(const char *str, const SDL_Color& col){
  SDL_Surface *tmp = TTF_RenderText_Blended(font, str, 0, col);
  if(!tmp){
    std::cerr << "Error rendering text: " << SDL_GetError() << std::endl;
    return nullptr;
  }
  return tmp;
}

bool Glyphs::table_allocate(Renderer& renderer){
  if(!font){
    std::cerr << "Invalid font parameter" << std::endl;
    return false;
  }

  for(size_t c = 0; c < colours.size(); c++){
    std::array<Glyph_Entry, ASCII_SIZE>& glyph_array = glyph_table[c];
    for(u8 i = ASCII_START; i < ASCII_SIZE; i++){
      Glyph_Entry& e = glyph_array[i];    
      const char charstr[] = { (char)i, NULLCHAR };

      SDL_Surface *surf = create_glyph_surface(charstr, colours[c]);
      if(!surf){
        std::cerr << "Failed to generate table" << std::endl;
        return false;
      }
      SDL_Texture *texture = renderer.create_texture(surf);
      if(!texture){
        std::cerr << "Failed to generate table" << std::endl;
        return false;
      }

      const i32 width = surf->w;
      const i32 height = surf->h;
      
      e.width = width;
      e.height = height;
      e.texture = texture;
      e.c = i;

      SDL_DestroySurface(surf);
    }
  }
  std::cerr << "Generated " << colours.size() * (ASCII_SIZE - ASCII_START) << " characters" << std::endl;
  return true;
}

void Glyphs::table_deallocate(void) {
  for(size_t c = 0; c < colours.size(); c++){
    std::array<Glyph_Entry, ASCII_SIZE>& glyph_array = glyph_table[c];
    for(u8 i = ASCII_START; i < ASCII_SIZE; i++){
      Glyph_Entry& e = glyph_array[i];    
      SDL_DestroyTexture(e.texture);
      e.texture = nullptr;
    }
  }
}



