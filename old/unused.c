#include "../include/typedef.h"
#include "../include/waveform.h"

#include <math.h>

// UNUSED (FOR LEARNING PURPOSES)
const i32 TRIANGLE_HARMONIC_MAX = 25;
const i32 SAW_HARMONIC_MAX = 50;
const i32 SQUARE_HARMONIC_MAX = 40;
//https://en.wikipedia.org/wiki/Pulse_wave
f64 fourier_pulse(f64 phase, f64 duty){
    f64 sum = 0.0;
    for(i32 n = 1; n <= SQUARE_HARMONIC_MAX; n++){
        sum += (1.0 / n) 
        * sin(PI * n * duty) 
        * cos(2.0 * PI * n * phase);
    }
    return duty + (2.0 / PI) * sum;

}

//https://en.wikipedia.org/wiki/Square_wave_(waveform)
f64 fourier_square(f64 phase){
    f64 sum = 0.0;
    for(i32 k = 1; k <= SQUARE_HARMONIC_MAX; k++){
        f64 n = 2.0 * k - 1.0;
        sum += (1.0 / n) * sin(2.0 * PI * n * phase);
    }
    return sum * (4.0 / PI);
}

f64 fourier_sawtooth(f64 phase){
    f64 sum = 0.0;
    for(i32 k = 1; k <= SAW_HARMONIC_MAX; k++){
        sum += pow(-1.0, k) * sin(2.0 * PI * k * phase) / k;
    }
    return sum * -(2 * 1.0 / PI);
}

f64 reverse_fourier_sawtooth(f64 phase){
    f64 sum = 0.0;
    for(i32 k = 1; k <= SAW_HARMONIC_MAX; k++){
        sum += pow(-1.0, k) * sin(2.0 * PI * k * phase) / k;
    }
    return sum * (2 * 1.0 / PI);
}