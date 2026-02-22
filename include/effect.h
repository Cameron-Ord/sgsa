#ifndef EFFECT_H
#define EFFECT_H
#include "typedef.h"
#include <stdbool.h>
#include <stddef.h>

struct delay_line {
    bool active;
    size_t start, end;
    f32* buffer;
    size_t read, write;
};

void print_delay_line(struct delay_line dl);
void delay_line_write(f32 sample, struct delay_line* dl);
f32 delay_line_read(struct delay_line* dl);
struct delay_line create_delay_line(size_t nmemb);

#endif