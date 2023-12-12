#include "Clipping.h"

bool lianBarsky(Scene& scene, Camera& camera, Line& line) // Line clipping
{
	int xmin = 0;
	int xmax = camera.horRes;
	int ymin = 0;
	int ymax = camera.verRes;
	
	return false;
}

bool sutherlandHodgman(Scene& scene, Camera& camera, Mesh& mesh) // Polygon clipping
{
	return false;
}
