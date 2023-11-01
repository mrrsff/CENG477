src = *.cpp

all:
	g++ $(src) -std=c++11 -O3 -o raytracer -pthread -Wl,--stack,16777216

simple:
	make all
	./raytracer.exe inputs/simple_shading.xml