#ifndef VIDEO_H
#define VIDEO_H

#include <SDL3/SDL_render.h>
#include "typedef.h"

#define RENDER_RESOLUTION (1 << 12)
#define WINDOW_HEIGHT 768
#define WINDOW_WIDTH 1024

struct render_context {
    SDL_Window *window;
    SDL_Renderer *renderer;
    u32 window_flags_at_creation;
    i32 win_width, win_height;
    SDL_Rect waveform_viewport;
    SDL_Rect opts_viewport;
    f32 buffer[RENDER_RESOLUTION];
};

struct glyph;

SDL_Rect make_rect(i32 x, i32 y, i32 w, i32 h);
void update_window_dims(struct render_context *rc);
void clear(SDL_Renderer *rend);
void set_colour(SDL_Renderer *rend, u8 r, u8 g, u8 b, u8 a);
void present(SDL_Renderer *rend);
void fill_rect(SDL_Renderer *rend, SDL_FRect *rect);
void draw_waveform(const struct render_context *rc);
void *free_texture(SDL_Texture *texture);
void *free_surface(SDL_Surface *surface);
void set_viewport(SDL_Renderer *rend, const SDL_Rect *viewport);
void draw_texture(SDL_Renderer *rend, SDL_FRect *src, SDL_FRect *dst, SDL_Texture *texture);
void draw_rect_at(SDL_Renderer *rend, f32 x, f32 y, f32 w, f32 h, SDL_Rect *viewport);
#endif