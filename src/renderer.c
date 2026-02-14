#include "../include/render.h"
#include "../include/waveform.h"
#include <stdio.h>

void render_set_sample_rects(SDL_Renderer *renderer, f32 samples[FRAME_RESOLUTION], i32 ww, i32 wh){
    const f32 HALF_HEIGHT = (f32)wh / 2;
    const f32 CELL_WIDTH = (f32)ww / FRAME_RESOLUTION;

    for(u32 i = 0; i < FRAME_RESOLUTION; i++){
        const f32 y = samples[i] * HALF_HEIGHT;
        const f32 x = (f32)i * CELL_WIDTH;

        SDL_FRect rect = {x, HALF_HEIGHT, CELL_WIDTH, y};
        render_set_rect(renderer, &rect);
    }
}

void render_set_blendmode(SDL_Renderer *renderer, u32 blendmode){
    SDL_SetRenderDrawBlendMode(renderer, blendmode);
}

void render_clear(SDL_Renderer *renderer){
    SDL_RenderClear(renderer);
}

void render_colour(SDL_Renderer *renderer, u8 r, u8 g, u8 b, u8 a){
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void render_present(SDL_Renderer *renderer){
    SDL_RenderPresent(renderer);
}

void render_set_rect(SDL_Renderer *renderer, SDL_FRect *rect){
    SDL_RenderFillRect(renderer, rect);
}