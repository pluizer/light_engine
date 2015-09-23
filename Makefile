FLAGS := `sdl2-config --libs --cflags` -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lGL -lGLEW

all:
	gcc -fPIC -shared *.c hypermath/*.c -o lible.so $(FLAGS)
