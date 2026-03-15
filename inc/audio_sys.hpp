#ifndef AUDIO_SYS_HPP
#define AUDIO_SYS_HPP
#include "define.hpp"
#include <SDL3/SDL.h>

void stream_get(void *data, SDL_AudioStream *stream, i32 add, i32 total);

class Audio_Sys {
public:
  Audio_Sys(i32 chan, i32 sample_rate);
  ~Audio_Sys(void) = default;
  bool open(void *userdata);
  void close(void);
  bool set_audio_callback(void *userdata);
  bool bind_stream(void);
  bool unbind_stream(void);
  bool open_audio_device(void);
  bool create_audio_stream(void);
  bool close_audio_device(void);
  bool destroy_audio_stream(void);
  bool resume(void);
  bool pause(void);
  void clear(void);

private:
  u32 dev;
  SDL_AudioStream *stream;
  SDL_AudioSpec internal;
  SDL_AudioSpec output;
};

#endif
