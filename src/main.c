
#include <SDL3/SDL_events.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "../include/controller.h"
#include "../include/waveform.h"
#include "../include/audio.h"
#include "../include/render.h"

#include <SDL3/SDL.h>

i32 window_width = 1024;
i32 window_height = 768;

const wave waveforms[] = { 
    triangle, 
    square, 
    fourier_sawtooth, 
    reverse_fourier_sawtooth, 
    sine 
};

const size_t waveform_count = sizeof(waveforms) / sizeof(waveforms[0]);
size_t waveform_index = 0;

static bool initialize_sdl(void);
static SDL_Renderer *create_renderer(SDL_Window *w);
static SDL_Window *create_window(const char *name, i32 width, i32 height);
static void show_window(SDL_Window *w);
static void destroy_window(SDL_Window *w);
static void destroy_renderer(SDL_Renderer *renderer);

static bool initialize_sdl(void){
    return SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO);
}

static void show_window(SDL_Window *w){
    SDL_ShowWindow(w);
}

static void destroy_window(SDL_Window *w){
    if(w){
        SDL_DestroyWindow(w);
    }
}

static void destroy_renderer(SDL_Renderer *renderer){
    if(renderer){
        SDL_DestroyRenderer(renderer);
    }
}

static SDL_Window *create_window(const char *name, i32 width, i32 height){
    SDL_Window *w = SDL_CreateWindow(name, width, height, SDL_WINDOW_HIDDEN);
    if(!w){
        printf("%s\n", SDL_GetError());
        return NULL;
    }
    return w;
}

static SDL_Renderer *create_renderer(SDL_Window *w){
    SDL_Renderer *r = SDL_CreateRenderer(w, NULL);
    if(!r){
        printf("%s\n", SDL_GetError());
        return NULL;
    }
    return r;
}

int main(int argc, char **argv){
    if(!initialize_sdl()){
        return 0;
    }

    u64 init_start = SDL_GetTicks();
    printf("Init start timer: %zums\n", init_start);

    SDL_Window *window = create_window("sgsa", window_width, window_height);
    if(!window){
        return 0;
    }

    SDL_Renderer *renderer =create_renderer(window);
    if(!renderer){
        return 0;
    }

    struct render_frame frame = {{0}};
    struct voice voices[VOICE_MAX];
    voices_initialize(voices, waveforms[waveform_index]);
    
    struct playback_device pbdev = open_audio_device(make_audio_spec(2, SAMPLE_RATE, SDL_AUDIO_F32));
    pbdev.stream = audio_stream_create(pbdev.output_spec, pbdev.output_spec);
    
    struct userdata data = {&frame, voices};

    pause_audio(pbdev.id);
    set_audio_callback(pbdev.stream, &data);
    audio_stream_bind(pbdev.stream, pbdev.id);
    resume_audio(pbdev.id);

    if(initialize_controller() < 0){
        return 0;
    }

    struct device_data device = get_first_input_controller();
    if(!device.valid){
        terminate_controller();
        return 0;
    }
    print_controller(&device);

    if(midi_open_stream(&device.stream, device.id, 512) < 0){
        terminate_controller();
        return 0;
    }    

    const u32 FPS = 60;
    const u32 FG = 1000 / FPS;
    bool RUNNING = true;

    printf("Init end timer: %zums : %zums\n", SDL_GetTicks(), SDL_GetTicks() - init_start);
    show_window(window);

    while(RUNNING){
        const u64 START = SDL_GetTicks();
        render_colour(renderer, 0, 0, 0, 255);
        render_clear(renderer);

        struct midi_input in = midi_read_input(device.stream, 1);
        switch(in.status){
            default: break;
            case NOTE_ON:{
                voice_set_iterate(voices, in.first, midi_to_base_freq(in.first));
            }break;

            case NOTE_OFF:{
                voice_release_iterate(voices, in.first);
            }break;
        }

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                default: break;

                case SDL_EVENT_KEY_DOWN:{
                    switch(event.key.key){
                        case SDLK_SPACE:{
                            size_t next = waveform_index + 1;
                            if(next >= waveform_count){
                                next = 0;
                            }
                            waveform_index = next;
                            voices_set_waveform(voices, waveforms[waveform_index]);
                        }break;
                    }
                }break;

                case SDL_EVENT_QUIT:{
                    RUNNING = false;
                }break;
            }
        }

        render_colour(renderer, 255, 255, 255, 255);
        render_set_sample_rects(renderer, frame.samples, window_width, window_height);
        render_present(renderer);

        const u64 FT = SDL_GetTicks() - START;
        if(FT < FG){
            const u32 DELAY = (u32)(FG - FT);
            SDL_Delay(DELAY);
        }
    }

    terminate_controller();
    close_audio_device(pbdev.id);
    audio_stream_destroy(pbdev.stream);
    destroy_window(window);
    destroy_renderer(renderer);
    SDL_Quit();
    return 1;
}