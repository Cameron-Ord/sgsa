# SGSA
### A dead simple configurable synthesizer.
> This is an extremely simple synth implementation, and basically serves as a template for other synths going forward.
- Built using PortMidi for midi input control with any midi controller and SDL3 for audio. This keeps things simple and cross platform.


### State of the application
- Configurable via lia - considering options with scripting going forward but im just using lua for config mainly. Might ditch it later
- All that's really going on is saturation options/low pass
- Uses additive fourier equations to generate band limited wave tables.

### Todo
- UI maybe? (Kind of necessary to be honest)
- Add effects like delay/reverb
