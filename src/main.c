
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stdio.h>

#include "../include/audio.h"
#include "../include/controller.h"
#include "../include/effect.h"
#include "../include/glyph.h"
#include "../include/util.h"
#include "../include/video.h"
#include "../include/waveform.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <SDL3/SDL.h>

static bool initialize_sdl(void);
static SDL_Renderer* create_renderer(SDL_Window* window);
static SDL_Window* create_window(const char* title, i32 width, i32 height, u32 flags);

static bool initialize_ttf(void) { return TTF_Init(); }

static bool initialize_sdl(void) { return SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS); }

static SDL_Renderer* create_renderer(SDL_Window* window) {
    SDL_Renderer* r = SDL_CreateRenderer(window, NULL);
    if (!r) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        return NULL;
    }
    return r;
}

static SDL_Window* create_window(const char* title, i32 width, i32 height, u32 flags) {
    SDL_Window* w = SDL_CreateWindow(title, width, height, flags);
    if (!w) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return NULL;
    }
    return w;
}

int main(int argc, char** argv) {
    const char* devname = NULL;
    if (argc > 1 && argc < 3) {
        devname = argv[1];
    } else {
        printf("Usage: sgsa {device name}\n");
        return 0;
    }
    assert(devname != NULL);
    srand((u32)time(NULL));

    if (initialize_controller() < 0) {
        return 0;
    }
    list_available_controllers();

    struct device_data device = get_input_controller(devname);
    if (!device.valid) {
        terminate_controller();
        return 0;
    }
    print_controller(&device);

    if (midi_open_stream(&device.stream, device.id, 512) < 0) {
        terminate_controller();
        return 0;
    }

    if (!initialize_sdl()) {
        return 1;
    }

    if (!initialize_ttf()) {
        return 1;
    }
    u64 init_start = SDL_GetTicks();
    printf("Init start timer: %zums\n", init_start);

    SDL_Window* window = create_window("sgsa", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_HIDDEN);
    if (!window) {
        return 1;
    }

    SDL_Renderer* renderer = create_renderer(window);
    if (!renderer) {
        return 1;
    }

    struct render_context rc = {.window = window,
                                .renderer = renderer,
                                .window_flags_at_creation = SDL_WINDOW_HIDDEN,
                                .win_width = WINDOW_WIDTH,
                                .win_height = WINDOW_HEIGHT,
                                .waveform_viewport = make_rect(0, 0, WINDOW_WIDTH, (i32)(WINDOW_HEIGHT * 0.25f)),
                                .opts_viewport =
                                    make_rect(0, (i32)(WINDOW_HEIGHT * 0.25f), WINDOW_WIDTH, (i32)(WINDOW_HEIGHT * 0.75f))};

    struct voice_control vc;
    vc_initialize(&vc);
    vc_assign_render_buffer(&vc, rc.buffer, RENDER_RESOLUTION);
    print_config(vc.cfg);

    SDL_AudioSpec internal_spec =
        make_audio_spec((i32)vc.cfg.entries[CHANNELS].value, (i32)vc.cfg.entries[SAMPLE_RATE].value);
    struct playback_device pbdev = open_audio_device();
    pbdev.stream = audio_stream_create(&internal_spec, &pbdev.output_spec);

    pause_audio(pbdev.id);
    set_audio_callback(pbdev.stream, &vc);
    audio_stream_bind(pbdev.stream, pbdev.id);
    resume_audio(pbdev.id);

    struct font f = create_font("assets/font.ttf", 16);
    if (!f.font) {
        return 1;
    }

    struct glyph glyphs[ASCII_SIZE];
    create_glyph_textures(rc.renderer, f.font, glyphs);

    const u32 FPS = 60;
    const u32 FG = 1000 / FPS;
    bool RUNNING = true;
    printf("Init end timer: %zums : %zums\n", SDL_GetTicks(), SDL_GetTicks() - init_start);
    SDL_ShowWindow(window);
    while (RUNNING) {
        const u64 START = SDL_GetTicks();
        set_colour(rc.renderer, 40, 42, 54, 255);
        clear(rc.renderer);
        struct midi_input in = midi_read_input(device.stream, 1);
        switch (in.status) {
        default:
            break;

        case NOTE_ON: {
            voice_set_iterate(vc.voices, map_velocity(in.second), in.first);
        } break;

        case NOTE_OFF: {
            voice_release_iterate(vc.voices, in.first);
        } break;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            default:
                break;
            case SDL_EVENT_QUIT: {
                RUNNING = false;
            } break;
            }
        }

        set_colour(rc.renderer, 80, 250, 123, 255);
        draw_waveform(&rc);
        present(rc.renderer);
        const u64 FT = SDL_GetTicks() - START;
        if (FT < FG) {
            const u32 DELAY = (u32)(FG - FT);
            SDL_Delay(DELAY);
        }
    }

    terminate_controller();
    close_audio_device(pbdev.id);
    audio_stream_destroy(pbdev.stream);
    SDL_Quit();
    return 1;
}