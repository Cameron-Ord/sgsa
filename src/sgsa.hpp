#ifndef SGSA_HPP
#define SGSA_HPP
#include <SDL3/SDL.h>

class Audio {
public:
    Audio(void);
    ~Audio(void);
private:
};

class Renderer {
public:
    Renderer(void);
    ~Renderer(void);
private:
};

class Window {
public:
    Window(void);
    ~Window(void);
private:
};

class Events {
public:
    Events(void);
    ~Events(void);
private:
};


class Manager {
public:
    Manager(void);
    ~Manager(void);
private:
    Audio audio;
    Window window;
    Renderer renderer;
    Events events;

};

#endif