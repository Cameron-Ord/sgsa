# SGSA
### A dead simple single oscilator 4 voice synthesizer meant to emulate the the GBA's sound hardware.
> This is an extremely simple synth implementation, and basically serves as a template for other synths going forward.

- Internally uses a sample rate of 13379 and derives waveforms from wavetables containing 32 samples.

> This implementation isn't really meant to sound "Good" the GBA's direct sound was pretty noisy too.

- Built using PortMidi for midi input control with any midi controller and SDL3 for audio. This keeps things simple and cross platform.