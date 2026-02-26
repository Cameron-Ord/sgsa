#include "sgsa.hpp"
#include <iostream>

Controller::Controller(void){
    list_available_controllers();
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