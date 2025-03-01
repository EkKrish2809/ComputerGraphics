#ifndef FIRST_SCENE_H
#define FIRST_SCENE_H

#include "scene.h"
#include "../common.h"

class first_scene : public scene
{
public:
    first_scene();
    ~first_scene();

    void update();
    void draw();
    bool init();
    void uninit();

    void render_ui();
    void process_events();
    void resize(int width, int height);
};
#endif // FIRST_SCENE_H