del ./bin/win32/*.obj
del *.exe

cl.exe /Fo".\\bin\\" /c /EHsc /I C:\glew\include main.cpp ^
window_manager.cpp ^
Logger.cpp ^
includes\imgui\imgui.cpp ^
includes\imgui\imgui_impl_opengl3.cpp ^
includes\imgui\imgui_impl_win32.cpp ^
includes\imgui\imgui_tables.cpp ^
includes\imgui\imgui_widgets.cpp ^
includes\imgui\imgui_demo.cpp ^
includes\imgui\imgui_draw.cpp 
@REM src\*.cpp ^

link /out:Engine.exe bin\*.obj /LIBPATH:C:\glew\lib\Release\x64 user32.lib gdi32.lib glew32.lib OpenGL32.lib /SUBSYSTEM:WINDOWS 