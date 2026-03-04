#include "audio.hpp"
#include "controller.hpp"
#include "util.hpp"
#include <iostream>

Controller::Controller(const char *name)
    : input_name(name), input_id(-1), stream(NULL), msgbuf() {
  memset(msgbuf, 0, INPUT_TYPE::INPUT_END * sizeof(i32));
  list_available_controllers();
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

void Controller::clear_msg_buf(void) {
  for (size_t i = 0; i < INPUT_TYPE::INPUT_END; i++) {
    msgbuf[i] = 0;
  }
}

void Controller::read_input(i32 len) {
  PmEvent event;
  i32 count = Pm_Read(stream, &event, len);
  if (count > 0) {
    msgbuf[INPUT_TYPE::STATUS] = Pm_MessageStatus(event.message);
    msgbuf[INPUT_TYPE::MSG_ONE] = Pm_MessageData1(event.message);
    msgbuf[INPUT_TYPE::MSG_TWO] = Pm_MessageData2(event.message);
  }
}
