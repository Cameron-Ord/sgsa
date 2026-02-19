TARGET = sgsa

SRCS = src/main.c
SRCS += src/controller.c
SRCS += src/audio.c
SRCS += src/waveform.c
SRCS += src/effect.c
SRCS += src/util.c
SRCS += src/video.c
SRCS += src/configs.c
SRCS += src/oscilators.c

CC = gcc
LFLAGS = -lm -lSDL3 -lSDL3_ttf -lportmidi
CFLAGS  = -Wall -Wextra -Wpedantic -O0 -std=c23
DEBUG_CFLAGS = -Wshadow -Wconversion -Wnull-dereference -Wdouble-promotion -g

all: $(TARGET)

windows: CC = x86_64-w64-mingw32-gcc
windows: all

linux: CC = gcc
linux: all

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o  $(TARGET) $(SRCS) $(LFLAGS) $(DEBUG_CFLAGS)

clean:
	rm $(TARGET)
