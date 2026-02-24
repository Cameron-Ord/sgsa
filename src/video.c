#include "../include/video.h"
#include "../include/glyph.h"
#include "../include/util.h"
#include "../include/waveform.h"
#include <stdio.h>

SDL_Rect make_waveform_view(i32 ww, i32 wh, i32 x, i32 y) {
  return make_rect(x, y, ww, DIM_MOD(wh, WAVEFORM_RECT_HEIGHT_MOD));
}

SDL_Rect make_opt_view(i32 ww, i32 wh, i32 x, i32 y) {
  return make_rect(x, y, DIM_MOD(ww, OPT_RECT_WIDTH_MOD),
                   DIM_MOD(wh, OPT_RECT_HEIGHT_MOD));
}

SDL_Rect make_rect(i32 x, i32 y, i32 w, i32 h) {
  return (SDL_Rect){ x, y, w, h };
}

void set_viewport(SDL_Renderer *rend, const SDL_Rect *viewport) {
  SDL_SetRenderViewport(rend, viewport);
}

void draw_waveform(const struct render_context *rc, f32 *layer_window) {
  const SDL_Rect *vp = &rc->rect_meta_data[WAVEFORM_VIEWPORT_VAL];
  set_viewport(rc->renderer, vp);

  const f32 cell_width = (f32)vp->w / (f32)WINDOW_RESOLUTION;
  for (u32 i = 0; i < WINDOW_RESOLUTION; i++) {
    const f32 x = (f32)i * cell_width + cell_width / 2;
    const f32 y = layer_window[i] * (f32)vp->h;
    SDL_FRect rect = { x, (f32)vp->h / 2, cell_width, y };
    fill_rect(rc->renderer, &rect);
  }
}

void draw_rect_at(SDL_Renderer *rend, const f32 x, const f32 y, const f32 w,
                  const f32 h, SDL_Rect *viewport) {
  if (viewport) {
    set_viewport(rend, viewport);
  }
  SDL_FRect rect = { x, y, w, h };
  fill_rect(rend, &rect);
}

void draw_texture(SDL_Renderer *rend, SDL_FRect *src, SDL_FRect *dst,
                  SDL_Texture *texture) {
  SDL_RenderTexture(rend, texture, src, dst);
}

void update_window_dims(struct render_context *rc) {
  i32 *ww = &rc->integer_meta_data[WIN_WIDTH_VAL];
  i32 *wh = &rc->integer_meta_data[WIN_HEIGHT_VAL];
  SDL_GetWindowSize(rc->window, ww, wh);
}

void fill_rect(SDL_Renderer *rend, SDL_FRect *rect) {
  SDL_RenderFillRect(rend, rect);
}

void clear(SDL_Renderer *rend) { SDL_RenderClear(rend); }

void set_colour(SDL_Renderer *rend, u8 r, u8 g, u8 b, u8 a) {
  SDL_SetRenderDrawColor(rend, r, g, b, a);
}

void present(SDL_Renderer *rend) { SDL_RenderPresent(rend); }

void *free_texture(SDL_Texture *texture) {
  if (!texture)
    return NULL;
  SDL_DestroyTexture(texture);
  texture = NULL;
  return NULL;
}

void *free_surface(SDL_Surface *surface) {
  if (!surface)
    return NULL;
  SDL_DestroySurface(surface);
  surface = NULL;
  return NULL;
}
