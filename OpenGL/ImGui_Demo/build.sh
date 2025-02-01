rm 3Dshapes
rm *.o

g++ -c -I/usr/include -I./imgui -g OGL.cpp ./imgui/imgui.cpp ./imgui/imgui_impl_x11.cpp ./imgui/imgui_impl_opengl3.cpp ./imgui/imgui_draw.cpp ./imgui/imgui_demo.cpp ./imgui/imgui_widgets.cpp ./imgui/imgui_tables.cpp

g++ -o 3Dshapes  OGL.o imgui.o imgui_impl_x11.o imgui_impl_opengl3.o imgui_draw.o imgui_demo.o imgui_widgets.o imgui_tables.o -L/usr/lib/ -lX11 -lGL -lGLEW 
# g++ OGL.cpp ./imgui/imgui.cpp ./imgui/*.cpp  -lX11 -lGL -lGLEW

./3Dshapes
