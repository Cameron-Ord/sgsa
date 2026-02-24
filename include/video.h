#ifndef VIDEO_H
#define VIDEO_H

#include "typedef.h"
#include "waveform.h"
#include <SDL3/SDL_render.h>

#define WINDOW_HEIGHT 768
#define WINDOW_WIDTH  1024
// Too lazy to make these variables atm
#define WAVEFORM_RECT_HEIGHT_MOD 0.10f
#define OPT_RECT_HEIGHT_MOD      0.90f
#define OPT_RECT_WIDTH_MOD       0.50f
#define DIM_MOD(wh, mod)         (i32)((f32)(wh) * (mod))

enum video_data_indexes {
  WAVEFORM_VIEWPORT_VAL = 0,
  ENV_OPTS_VIEWPORT_VAL = 1,
  SPEC_OPTS_VIEWPORT_VAL = 2,
  VIDEO_RECTS_END = 3,

  WIN_WIDTH_VAL = 0,
  WIN_HEIGHT_VAL = 1,
  VIDEO_INTEGERS_END = 2,
};

struct render_context {
  SDL_Window *window;
  SDL_Renderer *renderer;
  u32 window_flags_at_creation;

  i32 integer_meta_data[VIDEO_INTEGERS_END];
  SDL_Rect rect_meta_data[VIDEO_RECTS_END];
};

struct glyph;
struct oscilator;

SDL_Rect make_waveform_view(i32 ww, i32 wh, i32 x, i32 y);
SDL_Rect make_opt_view(i32 ww, i32 wh, i32 x, i32 y);
SDL_Rect make_rect(i32 x, i32 y, i32 w, i32 h);

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