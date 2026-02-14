#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "../include/controller.h"
#include "../include/waveform.h"

int main(int argc, char **argv){
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

    struct voice voices[VOICE_MAX];
    voices_initialize(voices);

    while(true){
        struct midi_input in = midi_read_input(device.stream, 1);
        switch(in.status){
            default: break;

            case NOTE_ON:{
                voice_set_iterate(voices, in.first, midi_to_base_freq(in.first));
            }break;

            case NOTE_OFF:{
                voice_clear_iterate(voices, in.first);
            }break;
        }
    }

    terminate_controller();
    return 1;
}