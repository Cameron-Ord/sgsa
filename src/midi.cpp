#include "sgsa.hpp"
#include "util.hpp"
#include <iostream>

Controller::Controller(const char *name_arg) 
: input_name(), input_id(-1), stream(NULL), msgbuf() {
    memset(input_name, 0, CONTROLLER_NAME_MAX * sizeof(char));
    memset(msgbuf, 0, INPUT_END * sizeof(i32));
    list_available_controllers();
    get_midi_device_by_name(name_arg);
    open_stream(1024);
}

void Controller::iterate_input_off(struct Audio_Data& data, i32 midi_key){
    for(i32 i = 0; i < MAX_VOICE; i++){
        struct Voice& v = data.voices[i];
        if(check_bit(v.voice_state, VOICE_ON | ENVELOPE_RELEASING | ENVELOPE_OFF, VOICE_ON) && v.midi_key == midi_key){
            v.voice_state = set_bit(0, VOICE_OFF | ENVELOPE_RELEASING);
            return;
        }
    }
}

void Controller::iterate_input_on(struct Audio_Data& data, i32 midi_key){
    for(i32 i = 0; i < MAX_VOICE; i++){
        struct Voice& v = data.voices[i];
        if(check_bit(v.voice_state, VOICE_OFF | ENVELOPE_OFF | ENVELOPE_RELEASING | ENVELOPE_ATTACKING | ENVELOPE_DECAYING | ENVELOPE_SUSTAINING, VOICE_OFF | ENVELOPE_OFF)){
            memset(v.generative_states, 0, sizeof(f32) * STATE_END);
            memset(v.gen, 0, sizeof(f32) * CHANNEL_MAX);
            v.midi_key = midi_key;
            v.freq = midi_to_freq(midi_key);
            v.voice_state = set_bit(0, VOICE_ON | ENVELOPE_ATTACKING);
            return;
        }
    }
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

void Controller::clear_msg_buf(void){
    for(i32 i = 0; i < INPUT_END; i++){
        msgbuf[i] = 0;
    }
}

void Controller::read_input(i32 len){
    PmEvent event; 
    i32 count = Pm_Read(stream, &event, len);
    if(count > 0){
        msgbuf[INPUT_MSG_STATUS] = Pm_MessageStatus(event.message); 
        msgbuf[INPUT_MSG_ONE] = Pm_MessageData1(event.message); 
        msgbuf[INPUT_MSG_TWO] = Pm_MessageData2(event.message); 
    }
}