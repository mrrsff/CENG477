#include "Clipping.h"
#include <iostream>

bool visible(double p, double q, double &tE, double &tL) {
    double t = q / p;
    if (p < 0.0) {
        if (t > tL) return false;
        else if (t > tE) tE = t;
    } else if (p > 0.0) {
        if (t < tE) return false;
        else if (t < tL) tL = t;
    } else if (q < 0.0) return false;
    return true;
}

bool liangBarsky(Line& line) // Line clipping in canonical view volume (CVV)
{
	double xmin = -1, xmax = 1;
	double ymin = -1, ymax = 1;
	double zmin = -1, zmax = 1;

	double tE = 0, tL = 1; // tE = tEnter, tL = tLeave.
	// At the end of the algorithm, if tE > tL, the line is outside of the view frustum and should be discarded.

	double dx = line.p1.x - line.p0.x; // slope x
	double dy = line.p1.y - line.p0.y; // slope y
	double dz = line.p1.z - line.p0.z; // slope z

	Color dc = line.p1Color - line.p0Color; // slope color

	if (visible(dx, xmin - line.p0.x, tE, tL) && // left edge
		visible(-dx, line.p0.x - xmax, tE, tL) && // right edge
		visible(dy, ymin - line.p0.y, tE, tL) && // bottom edge 
		visible(-dy, line.p0.y - ymax, tE, tL) && // top edge
		visible(dz, zmin - line.p0.z, tE, tL) && // near edge
		visible(-dz, line.p0.z - zmax, tE, tL)) // far edge
	{
		if (tL < 1) // clips end of line because it is outside of the view frustum
		{
			line.p1.x = line.p0.x + tL * dx;
			line.p1.y = line.p0.y + tL * dy; 
			line.p1.z = line.p0.z + tL * dz;
			line.p1Color = line.p0Color + dc * tL;
		}
		if (tE > 0) // clips beginning of line because it is outside of the view frustum.
		{
			line.p0.x = line.p0.x + tE * dx;
			line.p0.y = line.p0.y + tE * dy;
			line.p0.z = line.p0.z + tE * dz;
			line.p0Color = line.p0Color + dc * tE;
		}
		return true; // line portion is visible and does not need to be clipped
	}
	return false;
}
