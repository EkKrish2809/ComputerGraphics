#ifndef RENDERER_H
#define RENDERER_H

#include "../common.h"
#include "scene.h"
#include "first_scene.h"

#include <vector>

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void render();
    void init();
    void uninit();
    void key_callback(int key, int scancode, int action, int mods);

private:
    scene *m_scene;
    std::vector<scene *> m_scenes;

};

#endif // RENDERER_H

/*

Main rendering -> Renderer.cpp and Renderer.h

scene structure : 
every scene related class will implement "scene" interface

effects structure :
create interface for effects
every effect will implement this interface

*/