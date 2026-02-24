#include "../include/video.h"
#include "../include/glyph.h"
#include "../include/util.h"
#include "../include/waveform.h"
#include <stdio.h>

void draw_cfgs(struct render_context *rc, struct glyph *glyphs, i32 lskip,
               struct osc_config *cfgs) {
  SDL_Rect *vp = &rc->rect_meta_data[ENV_OPTS_VIEWPORT_VAL];
  set_viewport(rc->renderer, vp);
  const f32 slider_width = 24.0f, slider_height = 16.0f;
  const f32 padding = 4.0f, x = padding;
  f32 y = padding;
  for (i32 i = 0; i < ENV_END; i++) {
    struct osc_entry_f32 *ent = &cfgs->env[i];
    draw_ascii_string(rc->renderer, ent->name, ent->name_len, glyphs, x, y);
    y = y + (f32)lskip + padding;

    
    const f32 slider_x = ((ent->value / ent->max_val) - (ent->min_val / 2.0f)) * (f32)vp->w;
    SDL_FRect slider = { x + slider_x, y, slider_width, slider_height };
    fill_rect(rc->renderer, &slider);
    y = y + slider_height + padding;
  }
  vp = &rc->rect_meta_data[SPEC_OPTS_VIEWPORT_VAL];
  set_viewport(rc->renderer, vp);
  y = 0.0f;
  for (i32 i = 0; i < SPEC_END; i++) {
    struct osc_entry_f32 *ent = &cfgs->spec[i];
    draw_ascii_string(rc->renderer, ent->name, ent->name_len, glyphs, x, y);
    y = y + (f32)lskip + padding;

    const f32 slider_x = ((ent->value / ent->max_val) - (ent->min_val / 2.0f)) * (f32)vp->w;
    SDL_FRect slider = { x + slider_x, y, slider_width, slider_height };
    fill_rect(rc->renderer, &slider);
    y = y + slider_height + padding;
  }
}

i32 apply_modifier(i32 val, f32 mod) { return (i32)((f32)val * mod); }

struct render_context make_render_context(u32 wflags, SDL_Window *w,
                                          SDL_Renderer *r, i32 ww, i32 wh) {
  const f32 OPT_HEIGHT_MOD = 0.50f;
  const f32 OPT_WIDTH_MOD = 0.5f;
  const f32 WAVEFORM_HEIGHT_MOD = 1.0f - OPT_HEIGHT_MOD;

  struct render_context rc = {
    .window = w,
    .renderer = r,
    .window_flags_at_creation = wflags,
    .float_meta_data = { [DMOD_HEIGHT_VAL] = OPT_HEIGHT_MOD,
                         [DMOD_WIDTH_VAL] = OPT_WIDTH_MOD,
                         [DMOD_WF_HEIGHT_VAL] = WAVEFORM_HEIGHT_MOD },
    .integer_meta_data = { [WIN_WIDTH_VAL] = ww, [WIN_HEIGHT_VAL] = wh },
    .rect_meta_data = { [WAVEFORM_VIEWPORT_VAL] = { 0, 0, 0, 0 },
                        [ENV_OPTS_VIEWPORT_VAL] = { 0, 0, 0, 0 },
                        [SPEC_OPTS_VIEWPORT_VAL] = { 0, 0, 0, 0 } }
  };

  SDL_Rect waveform =
   make_rect(0, 0, ww, apply_modifier(wh, WAVEFORM_HEIGHT_MOD));
  SDL_Rect spec = make_rect(0, apply_modifier(wh, WAVEFORM_HEIGHT_MOD),
                            apply_modifier(ww, OPT_WIDTH_MOD),
                            apply_modifier(wh, OPT_HEIGHT_MOD));
  SDL_Rect env = make_rect(
   apply_modifier(ww, OPT_WIDTH_MOD), apply_modifier(wh, WAVEFORM_HEIGHT_MOD),
   apply_modifier(ww, OPT_WIDTH_MOD), apply_modifier(wh, OPT_HEIGHT_MOD));

  rc.rect_meta_data[WAVEFORM_VIEWPORT_VAL] = waveform;
  rc.rect_meta_data[SPEC_OPTS_VIEWPORT_VAL] = spec;
  rc.rect_meta_data[ENV_OPTS_VIEWPORT_VAL] = env;

  return rc;
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
    const f32 y = layer_window[i] * ((f32)vp->h / 2.0f);
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
