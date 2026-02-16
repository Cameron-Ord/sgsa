
#include <SDL3/SDL_events.h>
#include <stdbool.h>
#include <stdio.h>

#include "../include/controller.h"
#include "../include/waveform.h"
#include "../include/audio.h"

#include <SDL3/SDL.h>
//https://github-wiki-see.page/m/pret/pokeemerald/wiki/Implementing-ipatix%27s-High-Quality-Audio-Mixer
const i32 INTERNAL_SAMPLE_RATE = 13379;

static bool initialize_sdl(void);

static bool initialize_sdl(void){
    return SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS);
}

int main(int argc, char **argv){
    //shutup
    (void)argc;
    (void)argv;

    if(!initialize_sdl()){
        return 0;
    }

    u64 init_start = SDL_GetTicks();
    printf("Init start timer: %zums\n", init_start);

    i32 current_waveform = POLY_PULSE;
    struct voice_control vc;
    vc_initialize(
        &vc,
        current_waveform,
        make_format(MONO, INTERNAL_SAMPLE_RATE, SDL_AUDIO_F32),
        make_osciliator(0.0, 0.0, 0.0, 0.0), 
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

    while(RUNNING){
        const u64 START = SDL_GetTicks();
        struct midi_input in = midi_read_input(device.stream, 1);
        switch(in.status){
            default: break;
            
            //this is quite nested, maybe do something about that
            case CONTROL:{
                switch(in.first){
                    default:break;
                    case WAVE_LEFT:{
                        switch(in.second){
                            default: break;
                            case CONTROL_ON:{
                                const i32 prev = prev_waveform(current_waveform);
                                vc_set_waveform(&vc, prev);
                                current_waveform = prev;
                            }break;
                        }
                    }break;
                    
                    case WAVE_RIGHT:{
                        switch(in.second){
                            default: break;
                            case CONTROL_ON:{
                                const i32 next = next_waveform(current_waveform);
                                vc_set_waveform(&vc, next);
                                current_waveform = next;
                            }break;
                        }
                    }break;
                }
            }break;

            case NOTE_ON:{
                voice_set_iterate(
                    vc.voices, 
                    in.first, 
                    make_osciliator(0.0, midi_to_base_freq(in.first), 0.0, map_velocity(in.second)), 
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