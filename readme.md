# GBA-Style 6-Voice Polyphonic Synth
A 6-voice polyphonic synthesizer designed to emulate the characteristic sound of the Nintendo GBA, using PortMidi for MIDI input and SDL3 for audio output.

> This project is not complete and is still in progress. It is not really at a stage that I want it to be yet

## Features
- 6 simultaneous voices – full polyphony for complex chords and melodies.
- GBA-style sound – low sample rate (13,379 Hz) for authentic lo-fi character.
- PolyBLEP oscillator – clean anti-aliased saw and square waves.
- MIDI controller support – play live with any MIDI keyboard.
- Cross-platform – built on SDL3 and PortMidi for broad compatibility.

## Audio Engine
- Sample rate: 13,379 Hz. Note that SDL3 resamples it to whatever the output device uses.
- Amplitude scaling: Designed for dynamic response based on MIDI velocity.
- Bit depth & quality: Uses floating-point samples; low sample rate naturally gives a lo-fi sound.
- Slight vibrato on notes held longer than 1 second.

## Dependencies
PortMidi
 – MIDI input handling.
SDL3
 – audio output.

## Notes
- This synth is a learning project designed to mimic GBA-era game audio, not modern high-fidelity synthesizers.
- The 13,379 Hz sample rate is intentional to capture the lo-fi character of the GBA hardware.
- PolyBLEP is used to avoid aliasing in saw and square waves even at low sample rates.

## License
- MIT License – feel free to use or modify.

## Todo
- Make things more customizable - use less constants for some stuff.
- Add triangle and noise channels to better emulate GBA channels 2–4.
- Add filter effects; like reverb, maybe.
- Since it uses SDL, possibly add an optional visualizer