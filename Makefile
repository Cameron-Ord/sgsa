TARGET = sgsa

SRCS = src/main.c
SRCS += src/controller.c

CC = x86_64-w64-mingw32-gcc
LFLAGS = -lm -lSDL3 -lportmidi
CFLAGS  = -Wall -Wextra -Wpedantic -O0 -std=c23
DEBUG_CFLAGS = -Wshadow -Wconversion -Wnull-dereference -Wdouble-promotion -g

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o  $(TARGET) $(SRCS) $(LFLAGS) $(DEBUG_CFLAGS)

clean:
	rm $(TARGET)
