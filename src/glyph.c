#include "../include/glyph.h"
#include "../include/video.h"
#include <stdio.h>

static void zero_glyph(struct glyph* g) {
    g->code = 0;
    g->width = 0;
    g->height = 0;
    g->texture = NULL;
}

static void set_glyph(struct glyph* g, i32 width, i32 height, u8 code, SDL_Texture* texture) {
    g->code = code;
    g->width = width;
    g->height = height;
    g->texture = texture;
}

static SDL_Surface* render_text_blended(TTF_Font* font, const char* c, size_t len, SDL_Color col) {
    SDL_Surface* surf = TTF_RenderText_Blended(font, c, len, col);
    if (!surf) {
        printf("Failed to create surface: %s\n", SDL_GetError());
        return NULL;
    }
    return surf;
}

static SDL_Texture* create_texture(SDL_Renderer* rend, SDL_Surface* surf) {
    SDL_Texture* texture = SDL_CreateTextureFromSurface(rend, surf);
    if (!texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return NULL;
    }
    return texture;
}

static bool is_printable_char(u8 code) {
    if (code >= 32 && code <= 127)
        return true;
    return false;
}

struct font create_font(const char* fontpath, f32 size) {
    TTF_Font* font = TTF_OpenFont(fontpath, size);
    if (!font) {
        printf("Failed to open font: %s\n", SDL_GetError());
        return (struct font){NULL, size, 0};
    }
    const i32 line_skip = TTF_GetFontLineSkip(font);
    printf("Line skip for font: %d\n", line_skip);
    return (struct font){font, size, line_skip};
}

struct glyph* get_glyph_at(struct glyph glyphs[], u8 code) {
    if (is_printable_char(code)) {
        return &glyphs[code];
    }
    return &glyphs['?'];
}

void create_glyph_textures(SDL_Renderer* rend, TTF_Font* font, struct glyph glyphs[]) {
    for (unsigned char i = ASCII_FIRST; i < ASCII_SIZE; i++) {
        const u8 chstr[] = {i, NULL_CHAR};
        struct glyph* g = &glyphs[i];
        zero_glyph(g);

        SDL_Surface* surface = render_text_blended(font, (const char*)chstr, 1, (SDL_Color){255, 255, 255, 255});
        if (!surface) {
            continue;
        }

        const u8 code = i;
        const i32 height = surface->h;
        const i32 width = surface->w;

        SDL_Texture* texture = create_texture(rend, surface);
        if (!texture) {
            surface = free_surface(surface);
            continue;
        }
        surface = free_surface(surface);

        set_glyph(g, width, height, code, texture);
    }
}

void draw_ascii_string(SDL_Renderer* rend, const SDL_Rect* viewport, const char* string, size_t len, struct glyph* glyphs,
                       f32 x, const f32 y) {
    if (viewport) {
        set_viewport(rend, viewport);
    }
    for (size_t i = 0; i < len; i++) {
        struct glyph* g = get_glyph_at(glyphs, (u8)string[i]);
        if (g) {
            SDL_FRect rect = {(f32)g->width * x, y, (f32)g->width, (f32)g->height};
            draw_texture(rend, NULL, &rect, g->texture);
        }
        x++;
    }
}