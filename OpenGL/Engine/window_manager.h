#ifndef WINDOW_H
#define WINDOW_H

#ifdef __GNUC__
// X11 header (11 th version of 'X')
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // XVisualInfo struct
#include <X11/XKBlib.h> // for KeyBoard input
#else
// #pragma once
#include <windows.h>
#include <windowsx.h>

#endif
// OpenGL header files
#include <GL/glew.h> //
#include <GL/gl.h>   // for OpenGL functionality

#ifdef __GNUC__
#include <GL/glx.h> // for bridging API's
#endif

#ifndef __GNUC__
// #pragma comment(lib, "glew32.lib")
// #pragma comment(lib, "OpenGL32.lib")
#endif

#include <memory.h> // for memset()

// IMGUI header files
#include "includes/imgui/imgui.h"
#include "includes/imgui/imgui_impl_opengl3.h"
#ifdef __GNUC__
#include "includes/imgui/imgui_impl_x11.h"
#else
#include "includes/imgui/imgui_impl_win32.h"
#endif

// logger header files
#include "Logger.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#ifdef __GNUC__
    extern IMGUI_IMPL_API int ImGui_ImplX11_EventHandler(XEvent &event, XEvent *next_event);
#else
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
extern logger g_logger;

class window_manager
{
public:
    window_manager();
    window_manager(int width, int height);
    ~window_manager();

    #ifdef __GNUC__
    int initialize();
    #else
    int initialize(HINSTANCE, HINSTANCE, LPSTR, int);
    #endif
    int initGL();
    void render();
    #ifdef __GNUC__
    void game_loop();
    void process_events();
    #else
    MSG game_loop();
    LRESULT process_events(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
    #endif
    void toggle_fullscreen();
    void resize(int width, int height);
    void uninitialize();

    void set_width(int width);
    void set_height(int height);
    int get_width();
    int get_height();

#ifdef __GNUC__
// BOOL bActiceWindow = FALSE;
#else
BOOL bActiceWindow = FALSE;
#endif

private:
#ifdef __GNUC__
    Display *display = NULL;
    XVisualInfo *visualInfo = NULL;
    Colormap colorMap;
    Window window;
    Bool fullScreen = False;

    GLXContext glxContext; // OpenGL related
    Bool bActiceWindow = False;

    int win_width;
    int win_height;
    Bool bDone = False;
    // local variables
    int defaultScreen;
    int defaultDepth;
    // PP variables
    GLXFBConfig *glxFBConfigs = NULL;
    GLXFBConfig bestGLXFBConfig;
    XVisualInfo *tmpXVisualInfo = NULL;
    int numFBConfig;

    XSetWindowAttributes windowAttribute;
    int styleMask;
    Atom wm_delete_window_atom;
    XEvent event;
    KeySym keysym;
    int screenWidth;
    int screenHeight;
    char keys[26];
#else
    HWND ghwnd = NULL;
    HWND hwnd;
    HDC ghdc;
    HGLRC ghrc;
    WNDCLASSEX wndclass;
	MSG msg;
	
	BOOL bDone = FALSE;
    int iHeightOfWindow, iWidthOfWindow;
    
    BOOL gbFullScreen = FALSE;
#endif
};

// window_manager m_window;

#endif // WINDOW_H