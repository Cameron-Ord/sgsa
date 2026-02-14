#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "../include/controller.h"


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

    while(true){
        struct midi_input in = midi_read_input(device.stream, 1);
        switch(in.status){
            default: break;

            case NOTE_ON:{
                printf("%d\n", in.first);
            }break;

            case NOTE_OFF:{
                printf("%d\n", in.first);
            }break;
        }
    }

    terminate_controller();
    return 1;
}