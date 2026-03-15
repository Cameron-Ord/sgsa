TARGET = sgsa
CC = x86_64-w64-mingw32-g++
LFLAGS = -lm -lSDL3 -lSDL3_ttf -lportmidi 
CFLAGS  = -Wall -Wextra -Wpedantic -O0 -std=c++17
DEBUG_CFLAGS = -Wshadow -Wconversion -Wnull-dereference -Wdouble-promotion -g

SRCS = src/main.cpp
SRCS += src/core/util.cpp
SRCS += src/core/midi.cpp
SRCS += src/core/audio.cpp
SRCS += src/core/synth.cpp
SRCS += src/core/oscillator.cpp
SRCS += src/core/voice.cpp
SRCS += src/core/filter.cpp
SRCS += src/core/generator.cpp
SRCS += src/core/delay.cpp
SRCS += src/core/modulations.cpp

SRCS += src/frontend/context.cpp
SRCS += src/frontend/renderer.cpp
SRCS += src/frontend/glyph.cpp
SRCS += src/frontend/events.cpp

all: $(TARGET)

windows: CC = x86_64-w64-mingw32-g++
windows: all

linux: CC = g++
linux: all

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o  $(TARGET) $(SRCS) $(LFLAGS) $(DEBUG_CFLAGS)

clean:
	rm $(TARGET)
