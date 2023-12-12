#ifndef _CLIPPING_H_
#define _CLIPPING_H_
#include "Line.h"
#include "Scene.h"

bool liangBarsky(Scene& scene, Camera& camera, Line& line); // Line clipping
bool sutherlandHodgman(Scene& scene, Camera& camera, Mesh& mesh); // Polygon clipping
#endif