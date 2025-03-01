#include "window_manager.h"

#ifdef __GNUC__
	typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig, GLXContext, Bool, const int *);
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
	GLXFBConfig glxFBConfig;
#else
#include <WinUser.h>
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
#endif

window_manager::window_manager()
{
}

window_manager::window_manager(int width, int height)
{
}

window_manager::~window_manager()
{
}

#ifdef __GNUC__
int window_manager::initialize()
{
    g_logger.init_logger("./logs/log.txt", "./logs/shader.txt");

    // display struct is availed by XServer to us
	display = XOpenDisplay(NULL);
	if (display == NULL)	
	{
		g_logger.log(logger::LogType::_ERROR_, "ERROR : XOpenDisplay() Failed !!!\n");
		uninitialize();
		exit(1);
	}

	//
	defaultScreen = XDefaultScreen(display);

	// 
	defaultDepth = XDefaultDepth(display, defaultScreen);

    // frame buffer attributes
    static int frameBufferAttributes[] = 
			{
				GLX_DOUBLEBUFFER, True,
				GLX_X_RENDERABLE, True, // context becomes hardware renderable
				GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT, // where we have to draw -> on window (GLX_WINDOW_BIT)
				GLX_RENDER_TYPE, GLX_RGBA_BIT, // RGBA type image (frame)
				GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
				GLX_RED_SIZE, 8,
				GLX_GREEN_SIZE, 8,
				GLX_BLUE_SIZE, 8,
				GLX_ALPHA_SIZE, 8,
				GLX_STENCIL_SIZE, 8,
				GLX_DEPTH_SIZE, 24,
				None
			}; // like pfd in Windows OS

	// choosing best visualInfo
	glxFBConfigs = glXChooseFBConfig(display, defaultScreen, frameBufferAttributes, &numFBConfig); // returns array of FB configs and its count
	if (glxFBConfigs == NULL)
	{
		g_logger.log(logger::LogType::_ERROR_, "ERROR : glXChooseFBConfig() Failed\n");
		uninitialize();
		exit(1);
	}
	g_logger.log(logger::LogType::_INFO_, "Found number of FBConfigs are %d\n", numFBConfig);

	// find best FBConfig from the aray
	int bestFrameBufferConfig = -1;
	int worstFrameBufferConfig = -1;
	int bestNumberOfSamples = -1;
	int worstNumberOfSamples = 999;

	for (int i = 0; i < numFBConfig; i++)
	{
		tmpXVisualInfo = glXGetVisualFromFBConfig(display, glxFBConfigs[i]);
		if (tmpXVisualInfo != NULL)
		{
			int samples, sampleBuffers;
			glXGetFBConfigAttrib(display, glxFBConfigs[i], GLX_SAMPLE_BUFFERS, &sampleBuffers);
			glXGetFBConfigAttrib(display, glxFBConfigs[i], GLX_SAMPLES, &samples);
	
			// fprintf(gpFile, "VisualInfo = 0x%lu found SampleBuffers = %d and samples = %d\n", tmpXVisualInfo->visualid, sampleBuffers, samples);
			g_logger.log(logger::LogType::_INFO_, "VisualInfo = 0x%lu found SampleBuffers = %d and samples = %d\n", tmpXVisualInfo->visualid, sampleBuffers, samples);

			if (bestFrameBufferConfig < 0 || sampleBuffers && samples > bestNumberOfSamples)
			{
				bestFrameBufferConfig = i;
				bestNumberOfSamples = samples;
			}

			if (worstFrameBufferConfig < 0 || !sampleBuffers || samples < worstNumberOfSamples)
			{
				worstFrameBufferConfig = i;
				worstNumberOfSamples = samples;
			}
		}
		XFree(tmpXVisualInfo);
		tmpXVisualInfo = NULL;
	}
	
	bestGLXFBConfig = glxFBConfigs[bestFrameBufferConfig];
	glxFBConfig = bestGLXFBConfig;
	XFree(glxFBConfigs);
	glxFBConfigs = NULL;

	visualInfo = glXGetVisualFromFBConfig(display, bestGLXFBConfig);
	// fprintf(gpFile, "visualId of bestVisualInfo is 0x%lu\n", visualInfo->visualid);
	g_logger.log(logger::LogType::_INFO_, "visualId of bestVisualInfo is 0x%lu\n", visualInfo->visualid);


	// 
	memset(&windowAttribute, 0, sizeof(XSetWindowAttributes));
	windowAttribute.border_pixel = 0;
	windowAttribute.background_pixel = XBlackPixel(display, defaultScreen);
	windowAttribute.background_pixmap = 0; // bkg picture vapraychay ka asa meaning hoto
	windowAttribute.colormap = XCreateColormap(display, 
						RootWindow(display, visualInfo->screen),
						visualInfo->visual,
						AllocNone);
	windowAttribute.event_mask = ExposureMask | KeyPressMask| StructureNotifyMask | FocusChangeMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask; //

	// 
	colorMap = windowAttribute.colormap;

	//
	styleMask = CWBorderPixel | CWBackPixel | CWColormap | CWEventMask;

	//
	window = XCreateWindow(display,
			RootWindow(display, visualInfo->screen),
			100, // x
			100, // y
			WIN_WIDTH, // width
			WIN_HEIGHT, // height
			0, // border width
			visualInfo->depth,  // depth of window
			InputOutput,
			visualInfo->visual,
			styleMask,
			&windowAttribute); 
	if (!window)
	{
		// fprintf(gpFile,"ERROR : XCreateWindow() Failed !!!\n");
		g_logger.log(logger::LogType::_ERROR_, "ERROR : XCreateWindow() Failed !!!\n");
		uninitialize();
		exit(1);
	}
	
	//
	XStoreName(display, window, "Worthy");
	
	// for closing window ... Xwindow does not have close button by default, 
	// we ask for it to Window Manager(GNOME, KDE, etc)
	wm_delete_window_atom = XInternAtom(display, "WM_DELETE_WINDOW", True); // WM -> Window Manager

	XSetWMProtocols(display, window, &wm_delete_window_atom, 1);

	// showing window
	XMapWindow(display, window);

	// centering of Window
	screenWidth = XWidthOfScreen(XScreenOfDisplay(display, defaultScreen)); 
	screenHeight = XHeightOfScreen(XScreenOfDisplay(display, defaultScreen));
	XMoveWindow(display, window, screenWidth/2 - WIN_WIDTH/2, screenHeight/2 - WIN_HEIGHT/2);
	
	// initialize the OpenGL
	if (initGL() != 0)
	{
		g_logger.log(logger::LogType::_ERROR_, "ERROR : initGL() Failed !!!\n");
		uninitialize();
		exit(1);
	}
	
	// ********* imgui *********
	// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
	
	// Setup Dear ImGui style
    ImGui::StyleColorsDark();
	
	// Setup Platform/Renderer backends
    ImGui_ImplOpenGL3_Init("#version 460 core\n");
	ImGui_ImplX11_Init(display, (void*)window);
	
    return 0;
}


#else
int window_manager::initialize(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	TCHAR szAppName[] = TEXT("MyWindow");

	// code
	g_logger.init_logger("./logs/log.txt", "./logs/shader.txt");

	// Initializaion of wndclassex structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	// wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hIcon = LoadIcon(hInstance, NULL);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	// wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hIconSm = LoadIcon(hInstance, NULL);

	/* Register Above wndclass */
	RegisterClassEx(&wndclass);

	iHeightOfWindow = GetSystemMetrics(SM_CYSCREEN); // Height of Window Screen
	iWidthOfWindow = GetSystemMetrics(SM_CXSCREEN);	 // Width Of Window Screen

	/* Create Window */
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName,
						  TEXT("Worthy"),
						  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
						  (iWidthOfWindow - WIN_WIDTH) / 2,
						  (iHeightOfWindow - WIN_HEIGHT) / 2,
						  WIN_WIDTH,
						  WIN_HEIGHT,
						  NULL,
						  NULL,
						  hInstance,
						  NULL);

	ghwnd = hwnd;
	
	ShowWindow(hwnd, iCmdShow);

	/* fore grounding and focusing window */
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// initialize the OpenGL
	if (initGL() != 0)
	{
		g_logger.log(logger::LogType::_ERROR_, "ERROR : initGL() Failed !!!\n");
		uninitialize();
		exit(1);
	}

	// ********* imgui *********
	// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
    ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
    ImGui_ImplOpenGL3_Init("#version 460 core\n");
	ImGui_ImplWin32_Init((void*)hwnd);
    return 0;
}
#endif

int window_manager::initGL()
{
	// code
	#ifdef __GNUC__
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte *)"glXCreateContextAttribsARB");
	if (glXCreateContextAttribsARB == NULL)
	{
		g_logger.log(logger::LogType::_ERROR_, "glXGetProcAddressARB() Failed\n");
		uninitialize();
		exit(1);
	}

	// version = 4.5, then 4 is major version and 6 is minor version
	GLint contextAttributes[] = {
				GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
				GLX_CONTEXT_MINOR_VERSION_ARB, 6,
				GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
				None
	};

	// fallback
	glxContext = glXCreateContextAttribsARB(display, glxFBConfig, NULL, True, contextAttributes);
	if (!glxContext)
	{
	
		GLint contextAttributes[] = {
                			GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
                			GLX_CONTEXT_MINOR_VERSION_ARB, 0,
                			None
                };
		glxContext = glXCreateContextAttribsARB(display, glxFBConfig, NULL, True, contextAttributes);

		g_logger.log(logger::LogType::_ERROR_, "Cannot support 4.6, and hence falling back to default varsion\n");
		return -1;

	}
	else
	{
		g_logger.log(logger::LogType::_INFO_, "Found the Support to 4.6 version\n");
	}

	// checking if Hardware rendering
	if (!glXIsDirect(display, glxContext))
	{
		g_logger.log(logger::LogType::_ERROR_, "Direct Rendering i.e. H.W. rendering not supported\n");
		return -2;
	}
	else
	{
		g_logger.log(logger::LogType::_INFO_, "Direct Rendering i.e. H.W. rendering supported\n");
	}

	// make it as current context
	glXMakeCurrent(display, window, glxContext);
	#else
	// windows related code
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	/* code */
	/* initialization of pixelformatdesciptor structure */
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR)); // memset((void*)&pfd , NULL, sizeof(OIXELFORAMTEDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	/* GetDC */
	ghdc = GetDC(ghwnd);

	/* Choose Pixel Format */
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);

	if (iPixelFormatIndex == 0)
		return -1;

	/* Set The choosen Puxel Format */
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
		return -2;

	/* binding API */
	/* Create OpenGL Rendering Context */
	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
		return -3;

	/* make the rendering as current cintext */
	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
		return -4;
	#endif
	// GLEW initialization
	if (glewInit() != GLEW_OK)
	{
		return(-3);
	}

	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	g_logger.log(logger::LogType::_INFO_, "OpenGL Initialization successfull\n");
	return 0;
}

#ifdef __GNUC__
void window_manager::game_loop()
{
    while (bDone == False)
    {
		process_events();
        if (bActiceWindow == True)
        {
			// Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
			#ifdef __GNUC__
            ImGui_ImplX11_NewFrame();
			#else
			ImGui_ImplWin32_NewFrame();
			#endif
            ImGui::NewFrame();
			
            // update();
            // draw();
            render();
        }
    }
}

#else
MSG window_manager::game_loop()
{
	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (bActiceWindow == TRUE)
			{
				// Start the Dear ImGui frame
				ImGui_ImplOpenGL3_NewFrame();
				#ifdef __LINUX__
				ImGui_ImplX11_NewFrame();
				#else
				ImGui_ImplWin32_NewFrame();
				#endif
				ImGui::NewFrame();
				
				/* Render the seen */
				render();
				// updatetheseen
				// update();
			}
		}
	}
	return msg;
}
#endif

#ifdef __GNUC__
void window_manager::process_events()
{
    while (XPending(display)) // analogus to PeekMessage() in Windows OS
    // if (XPending(display) > 0) // analogus to PeekMessage() in Windows OS
    {
        XNextEvent(display, &event); // GetMessage()

        ImGui_ImplX11_EventHandler(event, &event);

        switch (event.type)
        {
        case MapNotify: // WM_CREATE in windows
            break;
        case FocusIn:
            bActiceWindow = True;
            break;
        case FocusOut:
            bActiceWindow = False;
            break;
        case KeyPress:
            keysym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, 0);
            switch (keysym)
            {
            case XK_Escape:
                bDone = True;
                break;
            }

            XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);
            switch (keys[0])
            {
            case 'F':
            case 'f':
                if (fullScreen == False)
                {
                    toggle_fullscreen();
                    fullScreen = True;
                }
                else
                {
                    toggle_fullscreen();
                    fullScreen = False;
                }
                break;
            }
            break;
        case ConfigureNotify:
            win_width = event.xconfigure.width;
            win_height = event.xconfigure.height;
            resize(win_width, win_height);
            break;
        case 33: // retuened by WM_DELETE_WINDOW_ATOM i.e. close button
            bDone = True;
            break;
        }
    }
}
#else
// windows related code
LRESULT window_manager::process_events(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// code
	switch (iMsg)
	{
	case WM_SETFOCUS:
		bActiceWindow= TRUE;
		break;

	case WM_CHAR:
		switch (wParam)
		{
		case 'f':
		case 'F':
            toggle_fullscreen();
			break;
		case 27:
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_SIZE:
        resize(WORD(lParam), HIWORD(lParam));
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_MOUSEMOVE:
	{
		float mouseX = LOWORD(lParam);
		float mouseY = HIWORD(lParam);

		// Send this to camera class
		// camera.mouseInputs(mouseX, mouseY);
	}
	break;

	case WM_MOUSEWHEEL:
	{
		short scrollDelta = GET_WHEEL_DELTA_WPARAM(wParam);

		// Use scrollValue and scrollDelta here
		// camera.mouseScroll(scrollDelta);
	}
	break;

	default:
		break;
	}
	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}
#endif

void window_manager::toggle_fullscreen()
{
	#ifdef __GNUC__
    // local variables declarations
	Atom wm_current_state_atom;
	Atom wm_fullscreen_state_atom;
	XEvent event;

	// code
	wm_current_state_atom = XInternAtom(display, "_NET_WM_STATE", False);
	wm_fullscreen_state_atom = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

	memset(&event, 0, sizeof(XEvent));
	event.type = ClientMessage;
	event.xclient.window = window;
	event.xclient.message_type = wm_current_state_atom;
	event.xclient.format = 32;
	event.xclient.data.l[0] = fullScreen ? 0 : 1;
	event.xclient.data.l[1] = wm_fullscreen_state_atom;

	// like sent and post messages in Windows OS
	XSendEvent(display,
			RootWindow(display, visualInfo->screen),
			False, // if this msg is for this window or for its child window
			SubstructureNotifyMask,
			&event);
	#else
	// windows related code
	// variable declartions
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;

	//	code
	wp.length = sizeof(WINDOWPLACEMENT);
	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);

			if (GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED); // nccalksize
			}

			ShowCursor(TRUE); // Show the cursor
			gbFullScreen = TRUE;
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wp);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
	}
	#endif
}

void window_manager::resize(int width, int height)
{
    // code
	if (height == 0)
		height = 1;	// to avoid divided 0(Illeegal Instruction) in future cause

	glViewport(0, 0, width, height); // screen cha konta bhag dakhvu... sadhya sagli screen dakhvli jat ahe pn he game need nusar customize karta yet.. eth width ai height he GL_SIZEI ya type che ahet ...resize la yetana te int ya type madhe yetat
	
	// perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void window_manager::render()
{
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// ImGui::ShowDemoWindow();

	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
	ImGui::Text("This is some useful text.");  
	ImGui::Text("FPS : %.12f", ImGui::GetIO().Framerate);               // Display some text (you can use a format strings too)
	ImGui::End();

	// Start the Dear ImGui frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	#ifdef __GNUC__
    glXSwapBuffers(display, window);
	#else
	// windows related code
	SwapBuffers(ghdc);
	#endif
}

void window_manager::uninitialize(){
    // code

	// Cleanup
    ImGui_ImplOpenGL3_Shutdown();
	#ifdef __GNUC__
    	ImGui_ImplX11_Shutdown();
	#else
		ImGui_ImplWin32_Shutdown();
	#endif
    ImGui::DestroyContext();

	#ifdef __GNUC__
    if (glxFBConfigs)
    {
        XFree(glxFBConfigs);
        glxFBConfigs = NULL;
    }

    if (glxContext)
    {
        glXMakeCurrent(display, 0, 0);
        glXDestroyContext(display, glxContext);
        glxContext = NULL;
    }

    if (window)
    {
        XDestroyWindow(display, window);
        window = 0;
    }

    if (colorMap)
    {
        XFreeColormap(display, colorMap);
        colorMap = 0;
    }

    if (visualInfo)
    {
        XFree(visualInfo);
        visualInfo = NULL;
    }

    if (display)
    {
        XCloseDisplay(display);
        display = NULL;
    }
	#else
	// windows related code
	#endif

    
}

/*
void window_manager::set_width(int width)
{
}

void window_manager::set_height(int height)
{
}

int window_manager::get_width()
{
}

int window_manager::get_height()
{
}

void window_manager::render()
{
}
*/