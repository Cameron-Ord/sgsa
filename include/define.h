#ifndef DEFINE_H
#define DEFINE_H

#define PI 3.1415926535897932384626433832795f
#define VOICE_MAX 8
#define OSCILATOR_MAX 4

#define MONO 1
#define STEREO 2
#define CHANNEL_MAX 2

#define MIDI_VELOCITY_MAX 127
#define MIDI_NOTE_MAX 127

#define NYQUIST(samplerate) ((samplerate) * 0.5f)
#define HZ_TO_RAD_PER_SEC(freq) (2.0f * PI * (freq))
#define CUTOFF_IN_SEC(radians) (1.0f / (radians))

enum WAVEFORM_IDS {
    WAVE_FORM_BEGIN = 0,
    SINE,
    PULSE_RAW,
    SAW_RAW,
    TRIANGLE_RAW,
    PULSE_POLY,
    SAW_POLY,
    TRIANGLE_POLY,
    WAVE_FORM_END,
};

enum ENVELOPE_STATES {
    ENVELOPE_ATTACK = 0,
    ENVELOPE_SUSTAIN,
    ENVELOPE_DECAY,
    ENVELOPE_RELEASE,
    ENVELOPE_OFF,
};

// (VALUE - VALUE) / SAMPLES
#define ATTACK_INCREMENT(samplerate, ATK) (1.0f - 0.0f) / (ATK * (samplerate))
#define DECAY_INCREMENT(samplerate, DEC, SUS) (1.0f - SUS) / (DEC * (samplerate))
#define RELEASE_INCREMENT(envelope, samplerate, REL) (envelope) / (REL * (samplerate))

#endif