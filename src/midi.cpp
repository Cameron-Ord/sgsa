#include "sgsa.hpp"
#include "util.hpp"
#include <iostream>

Controller::Controller(const char *name_arg) 
: input_name(), input_id(-1), stream(NULL) {
    list_available_controllers();
    get_midi_device_by_name(name_arg);
    open_stream(1024);
}

bool Controller::open_stream(i32 bufsize){
    if(Pm_OpenInput(&stream, input_id, NULL, bufsize, NULL, NULL) < 0){
        std::cerr << "Failed to open midi input stream" << std::endl;
        return false;
    }
    return true;
}

void Controller::list_available_controllers(void){
    const i32 count = Pm_CountDevices();
    if(count < 1){
        std::cout << "No devices" << std::endl;
        return;
    }

    for(i32 i = 0; i < count; i++){
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if(info){
            printf("%d: %s [%s] %s\n", 
                i, info->name, info->interf,
                info->input ? "input" : "output"
            );
        }
    }
}

void Controller::get_midi_device_by_name(const char *name){
    const i32 count = Pm_CountDevices();
    if(count < 1){
        std::cout << "No devices" << std::endl;
        return;
    }

    if(!name){
        std::cout << "Invalid name parameter" << std::endl;
        return;
    }

    for(i32 i = 0; i < count; i++){
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if((info && info->input) && strcmp(info->name, name) == 0){
            copy_char_buffer(info->name, input_name, strlen(info->name));
            input_id = i;
            std::cout << "Found device: " << name << std::endl;
            return;
        }
    }
    std::cout << "Failed to find specified device" << std::endl;
}