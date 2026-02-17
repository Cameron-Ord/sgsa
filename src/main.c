
#include <SDL3/SDL_events.h>
#include <stdbool.h>
#include <stdio.h>

#include "../include/controller.h"
#include "../include/waveform.h"
#include "../include/audio.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <SDL3/SDL.h>

//TODO: 
// I want this thing to be versatile
// So probably want to build some sort of UI with SDL
// That will allow me to change things like the ADSR values in app
// Or maybe even the samplerate, etc. So that every time I wanna hear 
// how something might sound im not constantly changing the src and recompiling.

//https://github-wiki-see.page/m/pret/pokeemerald/wiki/Implementing-ipatix%27s-High-Quality-Audio-Mixer
const i32 INTERNAL_SAMPLE_RATE = 13379;

static bool initialize_sdl(void);

static bool initialize_sdl(void){
    return SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS);
}

static u32 change_layer(i32 msg1, u32 layer_count, u32 current_layer){
    switch(msg1){
        default:break;
        case WAVE_LEFT:{
            return prev_layer(current_layer, layer_count);
        }break;
        
        case WAVE_RIGHT:{
            return next_layer(current_layer, layer_count);
        }break;
    }
    return current_layer;
}

int main(int argc, char **argv){
    const char *devname = NULL;
    if(argc > 1 && argc < 3){
        devname = argv[1];
    } else {
        printf("Usage: sgsa {device name}\n");
        return 0;
    }
    assert(devname != NULL);
    srand((u32)time(NULL));

    if(!initialize_sdl()){
        return 0;
    }

    u64 init_start = SDL_GetTicks();
    printf("Init start timer: %zums\n", init_start);

    const struct layer layers[] = {
        make_layer(1,
            make_oscilator(PULSE_RAW, make_wave_spec(1.0, 0.125, 1.0, 0.0))
        ),
        make_layer(1,
            make_oscilator(PULSE_RAW, make_wave_spec(1.0, 0.25, 1.0, 0.0))
        ),
        make_layer(1,
            make_oscilator(TRIANGLE_RAW, make_wave_spec(1.0, 0.0, 1.0, 0.0))
        ),
    };
    u32 current_layer = 0;
    const u32 layer_count = sizeof(layers) / sizeof(layers[0]);

    struct voice_control vc;
    vc_initialize(
        &vc,
        make_format(MONO, INTERNAL_SAMPLE_RATE, SDL_AUDIO_F32),
        layers[current_layer], 
        make_env(ENVELOPE_OFF, 0.0, 0.0)
    );

    SDL_AudioSpec internal_spec = make_audio_spec(vc.fmt.CHANNELS, vc.fmt.SAMPLE_RATE, vc.fmt.FORMAT);
    struct playback_device pbdev = open_audio_device();
    pbdev.stream = audio_stream_create(internal_spec, pbdev.output_spec);
    
    pause_audio(pbdev.id);
    set_audio_callback(pbdev.stream, &vc);
    audio_stream_bind(pbdev.stream, pbdev.id);
    resume_audio(pbdev.id);

    if(initialize_controller() < 0){
        return 0;
    }

    struct device_data device = get_input_controller(devname);
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

    while(RUNNING){
        const u64 START = SDL_GetTicks();
        struct midi_input in = midi_read_input(device.stream, 1);
        switch(in.status){
            default: break;
            case CONTROL:{
                switch(in.second){
                    default: break;
                    case CONTROL_ON:{
                        current_layer = change_layer(in.first, layer_count, current_layer);
                        print_layer("Changed layer",layers[current_layer]);
                    }break;
                }
            }break;

            case NOTE_ON:{
                voice_set_iterate(
                    vc.voices, 
                    map_velocity(in.second), 
                    in.first, 
                    set_layer_freq(layers[current_layer], midi_to_base_freq(in.first)), 
                    make_env(ENVELOPE_ATTACK, 0.0, 0.0)
                );
            }break;

            case NOTE_OFF:{
                voice_release_iterate(
                    vc.voices, 
                    in.first,
                    vc.fmt.SAMPLE_RATE
                );
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

    terminate_controller();
    close_audio_device(pbdev.id);
    audio_stream_destroy(pbdev.stream);
    SDL_Quit();
    return 1;
}