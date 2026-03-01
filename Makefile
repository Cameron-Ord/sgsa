TARGET = sgsa
CC = x86_64-w64-mingw32-g++
LFLAGS = -lm -lSDL3 -lportmidi
CFLAGS  = -Wall -Wextra -Wpedantic -O0 -std=c++17
DEBUG_CFLAGS = -Wshadow -Wconversion -Wnull-dereference -Wdouble-promotion -g

SRCS = src/main.cpp
SRCS += src/util.cpp
SRCS += src/midi.cpp
SRCS += src/audio.cpp
SRCS += src/config.cpp
SRCS += src/generate.cpp

all: $(TARGET)

windows: CC = x86_64-w64-mingw32-g++
windows: all

linux: CC = g++
linux: all

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o  $(TARGET) $(SRCS) $(LFLAGS) $(DEBUG_CFLAGS)

clean:
	rm $(TARGET)
