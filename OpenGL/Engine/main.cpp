#include "main.h"

#ifndef __GNUC__
#include "includes/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#endif

window_manager window;

#ifdef __GNUC__
int main(int argc, char *argv[])
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
#endif
{
    // local variables
    window = window_manager(WIN_WIDTH, WIN_HEIGHT);

    // code
    #ifdef __GNUC__
    if (window.initialize() != 0)
    #else
    if (window.initialize(hInstance, hPrevInstance, lpszCmdLine, iCmdShow) != 0)
    #endif
    {
        #ifdef __GNUC__
        printf("ERROR : window.initialize() Failed !!!\n");
        #else
        MessageBox(NULL, TEXT("ERROR : window.initialize() Failed !!!"), TEXT("ERROR"), MB_OK);
        #endif
        exit(1);
    }

    #ifdef __GNUC__
    window.game_loop();
    #else
    MSG msg = {0};
    msg = window.game_loop();
    #endif
    
    window.uninitialize();

    #ifdef __GNUC__
    return 0;
    #else
    return ((int)msg.wParam);
    #endif
}
#ifndef __GNUC__
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    if(ImGui_ImplWin32_WndProcHandler(hwnd, iMsg, wParam, lParam))
	{
		return true;
	}

    switch(iMsg)
    {
        case WM_CREATE:
            break;

        case WM_ERASEBKGND:
            return (0);

        default:
            break;
    }

	/* deletegates event to ImGUI */
	return window.process_events(hwnd, iMsg, wParam, lParam);
	// return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}
#endif