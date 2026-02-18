#ifndef VIDEO_H
#define VIDEO_H

#include <SDL3/SDL_render.h>
#include "typedef.h"

#define RENDER_RESOLUTION 1024
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

void update_window_dims(struct render_context *rc);
void clear(SDL_Renderer *rend);
void set_colour(SDL_Renderer *rend, u8 r, u8 g, u8 b, u8 a);
void present(SDL_Renderer *rend);
void fill_rect(SDL_Renderer *rend, SDL_FRect *rect);
void draw_waveform(const struct render_context *rc);

#endif