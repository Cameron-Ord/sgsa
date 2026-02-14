#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <stdbool.h>
#include "typedef.h"

#define DEV_NAME_MAX 256
#define DEV_INTERF_MAX 512

enum STATUS {
    NOTE_ON = 0x90,
    NOTE_OFF = 0x80
};

struct device_data {
    i32 id;
    void *stream;
    char name[DEV_NAME_MAX + 1];
    char interface[DEV_INTERF_MAX + 1];
    bool input;
    bool valid;
};

struct midi_input {
    i32 status;
    i32 first;
    i32 second;
    bool valid;
};

i32 initialize_controller(void);
i32 terminate_controller(void);
i32 midi_open_stream(void **stream, i32 id, i32 bufsize);
struct device_data get_first_input_controller(void);
void print_controller(const struct device_data *device);
struct midi_input midi_read_input(void *stream, i32 len);
f64 midi_to_base_freq(i32 n);

#endif