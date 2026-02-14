#ifndef RENDER_H
#define RENDER_H
#include <SDL3/SDL_render.h>
#include "typedef.h"

//eventually use these and fit it to the window
#define RENDER_WIDTH 768
#define RENDER_HEIGHT 576

#define FRAME_RESOLUTION 256

struct render_frame {
    f32 samples[FRAME_RESOLUTION];
};  

void render_set_sample_rects(SDL_Renderer *renderer, f32 samples[FRAME_RESOLUTION], i32 ww, i32 wh);
void render_set_blendmode(SDL_Renderer *renderer, u32 blendmode);
void render_clear(SDL_Renderer *renderer);
void render_colour(SDL_Renderer *renderer, u8 r, u8 g, u8 b, u8 a);
void render_present(SDL_Renderer *renderer);
void render_set_rect(SDL_Renderer *renderer, SDL_FRect *dst);

#endif