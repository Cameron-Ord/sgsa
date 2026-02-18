#include "../include/video.h"
#include <stdio.h>
void draw_waveform(const struct render_context *rc){
    SDL_Rect viewport = { 0, 0, rc->win_width, rc->win_height  / 2 };
    SDL_SetRenderViewport(rc->renderer, &viewport);
    const f32 cell_width = (f32)rc->win_width / RENDER_RESOLUTION;

    for(u32 i = 0; i < RENDER_RESOLUTION; i++){
        const f32 x = (f32)i * cell_width + cell_width / 2;
        const f32 y = rc->buffer[i] * (f32)viewport.h;
        SDL_FRect rect = { x, (f32)viewport.h / 2, cell_width, y};
        fill_rect(rc->renderer, & rect);
    }
}

void update_window_dims(struct render_context *rc){
    SDL_GetWindowSize(rc->window, &rc->win_width, &rc->win_height);
}

void fill_rect(SDL_Renderer *rend, SDL_FRect *rect){
    SDL_RenderFillRect(rend, rect);
}

void clear(SDL_Renderer *rend){
    SDL_RenderClear(rend);
}

void set_colour(SDL_Renderer *rend, u8 r, u8 g, u8 b, u8 a){
    SDL_SetRenderDrawColor(rend, r, g, b, a);
}

void present(SDL_Renderer *rend){
    SDL_RenderPresent(rend);
}

