#ifndef WINDOW_HPP
#define WINDOW_HPP
#include "typedef.hpp"
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <vector>
#include <array>
#include <string>

class Window;
class Renderer;

struct Param_Float {
  std::string param_name;
  const f32& value;
};

enum Screen_Positions : i32 {
  SCREEN_LEFT,
  SCREEN_RIGHT,
  SCREEN_SIZE,
};

enum Glyph_Table_Dims : size_t {
  ASCII_START = 32,
  ASCII_SIZE = 128,
  TABLE_SIZE = ASCII_SIZE - ASCII_START
};

struct Glyph_Entry {
  SDL_Texture *texture;
  i32 width, height;
  const u8 c;
};

class Glyphs {
public:
  Glyphs(std::string _file_path, f32 font_size);
  ~Glyphs(void);

  bool open(void);
  void get_line_skip(void);
  void table_allocate(void);
  void destroy_table(void);
  void destroy_font(void);

  Glyph_Entry& get_glyph(const u8 c);

private:
  std::array<Glyph_Entry, TABLE_SIZE> glyph_table;
  std::string file_path;
  f32 font_size;
  TTF_Font *font;
  i32 line_skip;
};

class Renderer {
public:
  Renderer(i32 _flags);
  ~Renderer(void);
  void destroy_renderer(void);
  bool create_renderer(void);
  SDL_Renderer *get_renderer(void) { return r; }
  void clear(void);
  void colour(u8 r, u8 g, u8 b, u8 a);
  void present(void);
  void render_float_list(std::vector<Param_Float> params, i32 viewport_index);

private:
  i32 flags;
  std::array<SDL_Rect, SCREEN_SIZE> viewports;
  SDL_Renderer *r;
};

class Window {
public:
  Window(i32 _flags);
  ~Window(void);
  void destroy_window(void);
  bool create_window(void);
  SDL_Window* get_window(void) { return w; }
  Renderer& get_rend_class(void) { return rend; }
  void update_size(void);
private:
  i32 flags;
  SDL_Window *w;
  i32 width, height;
  Renderer rend;
};

#endif
