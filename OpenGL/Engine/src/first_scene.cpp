#include "first_scene.h"


first_scene::first_scene()
{
}

first_scene::~first_scene()
{
}

void first_scene::update()
{
}

void first_scene::draw()
{
}


bool first_scene::init()
{
    g_logger.log(logger::LogType::_INFO_, "first_scene::init() called\n");
    return true;
}

void first_scene::uninit()
{
    g_logger.log(logger::LogType::_INFO_, "first_scene::uninit() called\n");
}

void first_scene::render_ui()
{
}

void first_scene::process_events()
{
}

void first_scene::resize(int width, int height)
{
}

