#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP
#include "typedef.hpp"
#include <portmidi.h>
#include <string>
#include <array>

enum EVENT_CONSTANTS : size_t {
  INPUT_BUFFER_MAX = 64
};

enum EVENT_TYPE : size_t {
  NOTE_ON = 0x90,
  NOTE_OFF = 0x80,
  PITCH_BEND = 0xE0,
  CONTROL = 0xB0,
  CONTROL_ON = 0x7F,
  CONTROL_OFF = 0x0,
};

struct Midi_Input_Msg {
  Midi_Input_Msg(i32 _status, i32 _msg1, i32 _msg2) : status(_status), msg1(_msg1), msg2(_msg2) {}
  i32 status;
  i32 msg1, msg2;
};

class Controller {
public:
  Controller(const char *name_arg);
  ~Controller(void) = default;

  bool open(void);
  bool close(void);
  void list_available_controllers(void);
  void get_midi_device_by_name(void);
  bool open_stream(i32 bufsize);
  bool close_stream(void);
  i32 read_input(void);
  void clear_msg_buf(void);
  const PmEvent *get_event_at(i32 pos) const;
  const std::array<PmEvent, INPUT_BUFFER_MAX>& get_input_buffer(void) const { return input_buffer; }
  Midi_Input_Msg parse_event(PmEvent event);
  f32 normalize_pitch_bend(i32 value);

private:
  std::string input_name;
  i32 input_id;
  PortMidiStream *stream;
  std::array<PmEvent, INPUT_BUFFER_MAX> input_buffer;
};

#endif
