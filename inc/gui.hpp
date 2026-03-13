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

enum Glyph_Extra_Params : size_t {
  COLOUR_BASE,
  COLOUR_COUNT,
};

enum Screen_Positions : i32 {
  SCREEN_LEFT,
  SCREEN_RIGHT,
  SCREEN_SIZE,
};

enum Glyph_Table_Dims : u8 {
  ASCII_START = 32,
  ASCII_SIZE = 128,
};

struct Glyph_Entry {
  Glyph_Entry(void);
  SDL_Texture *texture;
  i32 width, height;
  u8 c;
};

class Glyphs {
public:
  Glyphs(std::string _file_path, f32 font_size);
  ~Glyphs(void);

  bool open(void);
  void close(void);
  void find_line_skip(void);
  bool table_allocate(Renderer& renderer);
  void table_deallocate(void);
  i32 get_line_skip(void) const { return line_skip; }
  const Glyph_Entry *get_glyph_at(u8 c, size_t colour_index) const;

private:
  SDL_Surface *create_glyph_surface(const char *str, const SDL_Color& col);
  std::array<SDL_Color, COLOUR_COUNT> colours;
  std::array<std::array<Glyph_Entry, ASCII_SIZE>, COLOUR_COUNT> glyph_table;
  std::string file_path;
  f32 font_size;
  TTF_Font *font;
  i32 line_skip;
};

class Renderer {
public:
  Renderer(void);
  ~Renderer(void);
  
  void destroy_renderer(void);
  bool create_renderer(SDL_Window *w);
  SDL_Texture *create_texture(SDL_Surface *surf);
  void clear(void);
  void clear_colour(u8 r, u8 g, u8 b, u8 a);
  void present(void);
  void render_float_list(std::vector<Param_Float> params, i32 viewport_index);
  SDL_Renderer *get_renderer(void) { return r; }

private:
  std::array<SDL_Rect, SCREEN_SIZE> viewports;
  SDL_Renderer *r;
};

class Events {
public:
private:
};

class Window {
public:
  Window(size_t _window_flags, i32 _width, i32 _height);
  ~Window(void);
  void destroy_window(void);
  bool create_window(void);
  void update_size(void);
  SDL_Window* get_window(void) { return w; }
  Renderer& get_render_class(void) { return rend; }
  void hide_window(void);
  void show_window(void);

private:
  size_t flags;
  SDL_Window *w;
  i32 width, height;
  Renderer rend;
  Events win_events;
};

#endif
