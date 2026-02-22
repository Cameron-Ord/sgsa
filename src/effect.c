#include "../include/effect.h"
#include "../include/util.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
// Pass MS_BUFFSIZE() to this
struct delay_line create_delay_line(size_t nmemb){
    f32 *buffer = sgsa_malloc(nmemb, sizeof(f32));
    if(!buffer){
        return (struct delay_line){false, 0, 0, NULL, 0, 0};
    }
    memset(buffer, 0, sizeof(f32) * nmemb);
    return (struct delay_line){true, 0, nmemb, buffer, 0, 0};
}

void print_delay_line(struct delay_line dl){
    size_t cols = 12;
    for(size_t i = dl.start; i < dl.end; i++){
        printf("{%.12f}(%zu - %zu)", (f64)dl.buffer[i], dl.read, dl.write);
        if(i % cols  == 0){
            printf("\n");
        }
    }
}
 
struct delay_line increment_delay_line(size_t read_inc, size_t write_inc, struct delay_line dl){
    const size_t next_read = (dl.read + read_inc) %  dl.end;
    const size_t next_write = (dl.write + write_inc) % dl.end;
    return (struct delay_line){ dl.active, dl.start, dl.end, dl.buffer, next_read, next_write };
}

void delay_line_write(const f32 sample, struct delay_line *dl){
    dl->buffer[dl->write] = sample;
    dl->write = (dl->write + 1) % dl->end;
}

f32 delay_line_read(struct delay_line *dl){
    f32 out = dl->buffer[dl->read];
    dl->read = (dl->read + 1) % dl->end;
    return out;
}

