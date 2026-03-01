#include "sgsa.hpp"
#include "util.hpp"
#include <iostream>

Controller::Controller(const char *name) 
: input_name(name), input_id(-1), stream(NULL), msgbuf() {
    memset(msgbuf, 0, INPUT_END * sizeof(i32));
    list_available_controllers();
}

bool Controller::open(void){
  get_midi_device_by_name();
  if(input_id < 0) {
    return false;
  }
  return open_stream(1024);
}

bool Controller::close(void){
  return close_stream();
}

void Controller::iterate_input_off(struct Audio_Data& data, i32 midi_key){
    for(size_t i = 0; i < data.ap_.voicings; i++){
        struct Voice *v = &data.voices[i];
        if(v->active_oscilators > 0 && v->midi_key == midi_key){
            for(size_t o = 0; o < v->osc_count; o++){
              struct Oscilator *osc = &v->oscs[o];
              osc->env_state = ENVELOPE_RELEASING;
            }
            return;
        }
    }
}

void Controller::iterate_input_on(struct Audio_Data& data, i32 midi_key){
    for(size_t i = 0; i < data.ap_.voicings; i++){
        struct Voice *v = &data.voices[i];
        if(v->active_oscilators <= 0){
          v->active_oscilators = 0;
          v->midi_key = midi_key;
          v->freq = midi_to_freq(midi_key);

          for(size_t o = 0; o < v->osc_count; o++){
            struct Oscilator *osc = &v->oscs[o];
            memset(osc->gen_states, 0, sizeof(f32) * STATE_END);
            osc->samples = Interpolator();
            osc->env_state = ENVELOPE_ATTACKING;
            v->active_oscilators++;
          }
          return;
        } 
    }
}

bool Controller::open_stream(i32 bufsize){
  std::cout << "Opening device with given ID: " << input_id << std::endl;
  PmError err = Pm_OpenInput(&stream, input_id, NULL, bufsize, NULL, NULL);
    if(err < 0){
        std::cerr << "Failed to open midi input stream: " << Pm_GetErrorText(err) << std::endl;
        return false;
    }
    return true;
}

bool Controller::close_stream(void){
  if(stream){ return false; }
  PmError err = Pm_Close(stream);
  if(err < 0){
    std::cerr << "Failed to close stream: " << Pm_GetErrorText(err) << std::endl;
    return false;
  }
  input_id = -1;
  stream = NULL;
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

void Controller::get_midi_device_by_name(void){
    const i32 count = Pm_CountDevices();
    if(count < 1){
        std::cout << "No devices" << std::endl;
        return;
    }
    input_id = -1;
    for(i32 i = 0; i < count; i++){
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if((info && info->input) && strcmp(info->name, input_name.c_str()) == 0){
            input_id = i;
            std::cout << "Found device: " << input_name << std::endl;
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
