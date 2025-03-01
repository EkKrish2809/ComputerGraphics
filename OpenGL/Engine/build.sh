rm ./bin/linux/*.o

g++ -c -I/usr/include -I./imgui -g main.cpp Logger.cpp window_manager.cpp ./includes/imgui/imgui.cpp ./includes/imgui/imgui_impl_x11.cpp ./includes/imgui/imgui_impl_opengl3.cpp ./includes/imgui/imgui_draw.cpp ./includes/imgui/imgui_demo.cpp ./includes/imgui/imgui_widgets.cpp ./includes/imgui/imgui_tables.cpp

mv *.o ./bin/linux/

# g++ -o Engine  main.o Logger.o window_manager.o imgui.o imgui_impl_x11.o imgui_impl_opengl3.o imgui_draw.o imgui_demo.o imgui_widgets.o imgui_tables.o -L/usr/lib/ -lX11 -lGL -lGLEW 
g++ -o Engine  ./bin/linux/*.o  -L/usr/lib/ -lX11 -lGL -lGLEW 

./Engine
