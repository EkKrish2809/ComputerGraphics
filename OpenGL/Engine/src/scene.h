#ifndef SCENE_H
#define SCENE_H


class scene
{
public:
    
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual bool init() = 0;
    virtual void uninit() = 0;

    virtual void render_ui() = 0;
    virtual void process_events() = 0;
    virtual void resize(int width, int height) = 0;
};

#endif // SCENE_H