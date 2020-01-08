LIBS = -lglfw3 -lGL -ldl -lX11 -lpthread -lXrandr -lXxf86vm -lXinerama -lXcursor -std=c++11 -lassimp -lGLU -lglut
CFLAGS = -Wall -Iinclude
GLADH = glad.h
GLADC = glad.c
GLAD = glad.o $(GLADH)
model = model.h
mesh = mesh.h
filesystem = filesystem.h

main: main.cpp $(GLAD) $(shader) $(camera) $(mesh) $(model) $(filesystem)
		g++ -o main main.cpp $(GLAD) $(LIBS)
glad.o: $(GLADH)
	gcc -c $(GLADC)
