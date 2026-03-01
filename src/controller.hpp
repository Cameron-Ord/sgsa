#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP
#include "typedef.hpp"
#include <portmidi.h>
#include <string>

enum EVENT_TYPE : size_t {
  NOTE_ON = 0x90,
  NOTE_OFF = 0x80,
  CONTROL = 0xB0,
  CONTROL_ON = 0x7F,
  CONTROL_OFF = 0x0,
};

enum INPUT_TYPE : size_t {
    STATUS,
    MSG_ONE,
    MSG_TWO,
    INPUT_END
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
    void read_input(i32 len);
    void clear_msg_buf(void);
    void iterate_input_on(struct Audio_Data &data, i32 midi_key);
    void iterate_input_off(struct Audio_Data &data, i32 midi_key);

    const i32 *get_msgbuf(void) const { return msgbuf; }

private:
    std::string input_name;
    i32 input_id;
    PortMidiStream *stream;
    i32 msgbuf[INPUT_END];
};

#endif

