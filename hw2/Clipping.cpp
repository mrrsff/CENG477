#include "Clipping.h"

bool visible(double den, double num, int& tE, int& tL)
{
	double t;
	if (den > 0) // Potentially entering
	{
		t = num / den;
		if (t > tL)
			return false;
		else if (t > tE)
			tE = t;
	}
	else if (den < 0) // Potentially leaving
	{
		t = num / den;
		if (t < tE)
			return false;
		else if (t < tL)
			tL = t;
	}
	else if (num > 0) // Parallel and outside
		return false;
	return true;
}

bool liangBarsky(Scene& scene, Camera& camera, Line& line) // Line clipping
{
	double xmin = 0, xmax = camera.horRes;
	double ymin = 0, ymax = camera.verRes;
	double zmin = camera.near, zmax = camera.far;

	int tE = 0, tL = 1;

	double dx = line.p1.x - line.p0.x; // slope x
	double dy = line.p1.y - line.p0.y; // slope y
	double dz = line.p1.z - line.p0.z; // slope z

	if (visible(dx, xmin - line.p0.x, tE, tL) && // left edge
		visible(-dx, line.p0.x - xmax, tE, tL) && // right edge
		visible(dy, ymin - line.p0.y, tE, tL) && // bottom edge 
		visible(-dy, line.p0.y - ymax, tE, tL) && // top edge
		visible(dz, zmin - line.p0.z, tE, tL) && // near edge
		visible(-dz, line.p0.z - zmax, tE, tL)) // far edge
	{
		if (tL < 1) // line portion is visible and needs to be clipped because it is leaving
		{
			line.p1.x = line.p0.x + tL * dx;
			line.p1.y = line.p0.y + tL * dy; 
			line.p1.z = line.p0.z + tL * dz;
		}
		if (tE > 0) // line portion is visible and needs to be clipped because it is entering 
		{
			line.p0.x = line.p0.x + tE * dx;
			line.p0.y = line.p0.y + tE * dy;
			line.p0.z = line.p0.z + tE * dz;
		}
		
		return true; // line portion is visible and does not need to be clipped
	}
	return false;
}

bool sutherlandHodgman(Scene& scene, Camera& camera, Mesh& mesh) // Polygon clipping
{
	return false;
}
