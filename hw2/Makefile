src = *.cpp
files = *.ppm

all:
	g++ -fmax-errors=0 $(src) -std=c++11 -O3 -o rasterizer -pthread

clean:
	rm -f rasterizer