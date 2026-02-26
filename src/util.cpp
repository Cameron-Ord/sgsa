#include "util.hpp"
#include <cmath>

f32 buffer_rms(size_t count, f32 *buffer){
    f32 sum = 0.0f;
    for(size_t i = 0; i < count; i++){
        sum += buffer[i] * buffer[i];
    }
    return sqrtf(sum / (f32)count);
}