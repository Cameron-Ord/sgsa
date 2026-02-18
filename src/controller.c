#include "../include/controller.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <portmidi.h>

void list_available_controllers(void){
    const i32 dcount = Pm_CountDevices();
    if(dcount < 1) {
        printf("No devices found\n");
        return;
    }
    for(i32 i = 0; i < dcount; i++){
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if(info){
            printf("%d: %s [%s] %s\n",i, info->name, info->interf, info->input ? "input" : "output");
        }
    }
}

i32 terminate_controller(void){
    return Pm_Terminate();
}

i32 initialize_controller(void){
    return Pm_Initialize();
}

void print_controller(const struct device_data *device){
    if(!device) return;
    printf("Assigned controller: %d - %s - [%s]\n", device->id, device->name, device->interface);
}

static struct device_data make_device_data(i32 id, const char *name, const char *interf, i32 input){
    bool valid = (!(id < 0) && (name && interf) && input);
    struct device_data data;

    data.id = id;
    data.valid = valid;
    data.input = input;
    data.stream = NULL;

    memset(data.name, 0, sizeof(data.name));
    memset(data.interface, 0, sizeof(data.interface));

    if(name){
        const size_t len = strlen(name);
        strncpy(data.name, name, len);
        data.name[len] = NULLCHAR;
    }

    if(interf){
        const size_t len = strlen(interf);
        strncpy(data.interface, interf, len);
        data.interface[len] = NULLCHAR;
    }

    return data;
}

struct device_data get_input_controller(const char *name){
    const i32 dcount = Pm_CountDevices();
    for(i32 i = 0; i < dcount; i++){
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if((info && info->input) && strcmp(info->name, name) == 0){
            return make_device_data(i, info->name, info->interf, info->input);
        }
    }
    return make_device_data(-1, NULL, NULL, 0);
}

struct midi_input midi_read_input(PortMidiStream *stream, i32 len){
    PmEvent event;
    i32 count = Pm_Read(stream, &event, len);
    if(count > 0){
        return (struct midi_input){
            Pm_MessageStatus(event.message),
            Pm_MessageData1(event.message),
            Pm_MessageData2(event.message),
            true
        };
    }
    return (struct midi_input){
        0, 0, 0, false
    };
}

i32 midi_open_stream(PortMidiStream **stream, i32 id, i32 bufsize){
    return Pm_OpenInput(stream, id, NULL, bufsize, NULL, NULL);
}
