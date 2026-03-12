#include "../../inc/controller.hpp"
#include <iostream>
#include <cstring>
#include <cmath>

Controller::Controller(const char *name)
    : input_name(name), input_id(-1), stream(NULL), input_buffer() {
  clear_msg_buf();
  list_available_controllers();
}

f32 Controller::normalize_pitch_bend(i32 value){
  return ((f32)value - (128.0f / 2.0f)) / (128.0f / 2.0f);
}

bool Controller::open(void) {
  get_midi_device_by_name();
  if (input_id < 0) {
    return false;
  }
  return open_stream(1024);
}

bool Controller::close(void) { return close_stream(); }

bool Controller::open_stream(i32 bufsize) {
  std::cout << "Opening device with given ID: " << input_id << std::endl;
  PmError err = Pm_OpenInput(&stream, input_id, NULL, bufsize, NULL, NULL);
  if (err < 0) {
    std::cerr << "Failed to open midi input stream: " << Pm_GetErrorText(err)
              << std::endl;
    return false;
  }
  return true;
}

bool Controller::close_stream(void) {
  if (stream) {
    return false;
  }
  PmError err = Pm_Close(stream);
  if (err < 0) {
    std::cerr << "Failed to close stream: " << Pm_GetErrorText(err)
              << std::endl;
    return false;
  }
  input_id = -1;
  stream = NULL;
  return true;
}

void Controller::list_available_controllers(void) {
  const i32 count = Pm_CountDevices();
  if (count < 1) {
    std::cout << "No devices" << std::endl;
    return;
  }

  for (i32 i = 0; i < count; i++) {
    const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
    if (info) {
      printf("%d: %s [%s] %s\n", i, info->name, info->interf,
             info->input ? "input" : "output");
    }
  }
}

void Controller::get_midi_device_by_name(void) {
  const i32 count = Pm_CountDevices();
  if (count < 1) {
    std::cout << "No devices" << std::endl;
    return;
  }
  input_id = -1;
  for (i32 i = 0; i < count; i++) {
    const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
    if ((info && info->input) && strcmp(info->name, input_name.c_str()) == 0) {
      input_id = i;
      std::cout << "Found device: " << input_name << std::endl;
      return;
    }
  }
  std::cout << "Failed to find specified device" << std::endl;
}


Midi_Input_Msg Controller::parse_event(PmEvent event){
  return Midi_Input_Msg(
    Pm_MessageStatus(event.message),
    Pm_MessageData1(event.message),
    Pm_MessageData2(event.message)
  );
}

void Controller::clear_msg_buf(void) {
  for (size_t i = 0; i < input_buffer.size(); i++) {
    memset(&input_buffer[i], 0, sizeof(PmEvent));
  }
}

const PmEvent *Controller::get_event_at(i32 pos) const {
  if((size_t)pos >= input_buffer.size() || pos < 0){
    return nullptr;
  }
  return &input_buffer[(size_t)pos];
}

i32 Controller::read_input(void) {
  return Pm_Read(stream, input_buffer.data(), (i32)input_buffer.size());
}
