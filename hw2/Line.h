#ifndef _LINE_H_
#define _LINE_H_
#include "Vec3.h"
#include <iostream>

class Line
{
	public:
		Vec3 origin;
		Vec3 direction;
	
		Line();
		Line(Vec3 origin, Vec3 direction);
		Line(const Line &other);
		Line &operator=(const Line &other);
		friend std::ostream &operator<<(std::ostream &os, const Line &l);

		Vec3 getPoint(double t);
};
