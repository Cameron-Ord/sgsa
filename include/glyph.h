#ifndef GLYPH_H
#define GLYPH_H
#include "typedef.h"
#include <SDL3_ttf/SDL_ttf.h>

#define ASCII_SIZE  128
#define ASCII_LAST  127
#define ASCII_FIRST 32

#define NEW_LINE  '\n'
#define NULL_CHAR '\0'

struct font {
    TTF_Font *font;
    f32 size;
    i32 line_skip;
};

struct glyph {
    u8 code;
    SDL_Texture *texture;
    i32 width, height;
};

struct font create_font(const char *fontpath, f32 size);
void create_glyph_textures(SDL_Renderer *rend, TTF_Font *font,
                           struct glyph glyphs[]);
struct glyph *get_glyph_at(struct glyph glyphs[], u8 code);
void draw_ascii_string(SDL_Renderer *rend, const SDL_Rect *viewport,
                       const char *string, size_t len, struct glyph *glyphs,
                       f32 x, f32 y);
#endif