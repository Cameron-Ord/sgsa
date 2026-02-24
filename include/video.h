#ifndef VIDEO_H
#define VIDEO_H

#include "typedef.h"
#include "waveform.h"
#include <SDL3/SDL_render.h>

#define WINDOW_HEIGHT 768
#define WINDOW_WIDTH  1024
// Too lazy to make these variables atm

enum video_data_indexes {
  WAVEFORM_VIEWPORT_VAL = 0,
  ENV_OPTS_VIEWPORT_VAL = 1,
  SPEC_OPTS_VIEWPORT_VAL = 2,
  VIDEO_RECTS_END = 3,

  DMOD_HEIGHT_VAL = 0,
  DMOD_WIDTH_VAL = 1,
  DMOD_WF_HEIGHT_VAL = 2,
  DMOD_END = 3,

  WIN_WIDTH_VAL = 0,
  WIN_HEIGHT_VAL = 1,
  VIDEO_INTEGERS_END = 2,
};

struct render_context {
  SDL_Window *window;
  SDL_Renderer *renderer;
  u32 window_flags_at_creation;

  f32 float_meta_data[DMOD_END];
  i32 integer_meta_data[VIDEO_INTEGERS_END];
  SDL_Rect rect_meta_data[VIDEO_RECTS_END];
};

struct glyph;
struct oscilator;

struct render_context make_render_context(u32 wflags, SDL_Window *w, SDL_Renderer *r, i32 ww, i32 wh);
i32 apply_modifer(i32 val, f32 mod);
SDL_Rect make_rect(i32 x, i32 y, i32 w, i32 h);

void draw_cfgs(struct render_context *rc, struct glyph *glyphs, i32 lskip, struct osc_config *cfgs);
void update_window_dims(struct render_context *rc);
void clear(SDL_Renderer *rend);
void set_colour(SDL_Renderer *rend, u8 r, u8 g, u8 b, u8 a);
void present(SDL_Renderer *rend);
void fill_rect(SDL_Renderer *rend, SDL_FRect *rect);
void draw_waveform(const struct render_context *rc, f32 *layer_window);
void *free_texture(SDL_Texture *texture);
void *free_surface(SDL_Surface *surface);
void set_viewport(SDL_Renderer *rend, const SDL_Rect *viewport);
void draw_texture(SDL_Renderer *rend, SDL_FRect *src, SDL_FRect *dst,
                  SDL_Texture *texture);
void draw_rect_at(SDL_Renderer *rend, f32 x, f32 y, f32 w, f32 h,
                  SDL_Rect *viewport);
#endif