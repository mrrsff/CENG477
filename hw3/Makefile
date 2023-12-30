includes = -I. -I/usr/include/freetype2 -I/usr/include/libpng16 
links = -lfreetype -lglfw -lpthread -lX11 -ldl -lXrandr -lGLEW -lGL
flags = -g -DGL_SILENCE_DEPRECATION -DGLM_ENABLE_EXPERIMENTAL
output = main

all:
	g++ main.cpp -o $(output) $(includes) $(links) $(flags)

clean:
	rm -f main

run:
	make all
	./main