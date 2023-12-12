#ifndef _CLIPPING_H_
#define _CLIPPING_H_
#include "Line.h"
#include "Scene.h"

class Clipping
{
	public:
		static void liangBarsky(Scene& scene, Camera& camera, Line& line); // Line clipping
		static void sutherlandHodgman(Scene& scene, Camera& camera, Mesh& mesh); // Polygon clipping
};