TARGET = sgsa
CC = gcc
LFLAGS = -lm -lSDL3 -lSDL3_ttf -lportmidi
CFLAGS  = -Wall -Wextra -Wpedantic -O0 -std=c23
DEBUG_CFLAGS = -Wshadow -Wconversion -Wnull-dereference -Wdouble-promotion -g

SRCS = src/main.cpp


all: $(TARGET)

windows: CC = x86_64-w64-mingw32-gcc
windows: all

linux: CC = gcc
linux: all

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o  $(TARGET) $(SRCS) $(LFLAGS) $(DEBUG_CFLAGS)

clean:
	rm $(TARGET)