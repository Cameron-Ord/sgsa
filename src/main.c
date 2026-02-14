
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "../include/controller.h"
#include "../include/waveform.h"
#include "../include/audio.h"
#include <SDL3/SDL.h>

i32 window_width = 400;
i32 window_height = 300;

static bool initialize_sdl(void);
static SDL_Renderer *create_renderer(SDL_Window *w);
static SDL_Window *create_window(const char *name, i32 width, i32 height);

static bool initialize_sdl(void){
    return SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO);
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

    SDL_Window *window = create_window("sgsa", window_width, window_height);
    SDL_Renderer *renderer =create_renderer(window);

    struct voice voices[VOICE_MAX];
    voices_initialize(voices);

    struct playback_device pbdev = open_audio_device(make_audio_spec(2, SAMPLE_RATE, SDL_AUDIO_F32));
    pbdev.stream = audio_stream_create(pbdev.output_spec, pbdev.output_spec);
    
    pause_audio(pbdev.id);
    set_audio_callback(pbdev.stream, voices);
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

    while(RUNNING){
        const u64 START = SDL_GetTicks();

        struct midi_input in = midi_read_input(device.stream, 1);
        switch(in.status){
            default: break;
            case NOTE_ON:{
                printf("%d\n", in.first);
                voice_set_iterate(voices, in.first, midi_to_base_freq(in.first));
            }break;

            case NOTE_OFF:{
                voice_clear_iterate(voices, in.first);
            }break;
        }

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                default: break;
                case SDL_EVENT_QUIT:{
                    RUNNING = false;
                }break;
            }
        }

        const u64 FT = SDL_GetTicks() - START;
        if(FT < FG){
            const u32 DELAY = (u32)(FG - FT);
            SDL_Delay(DELAY);
        }
    }

    close_audio_device(pbdev.id);
    audio_stream_destroy(pbdev.stream);
    terminate_controller();
    SDL_Quit();
    return 1;
}