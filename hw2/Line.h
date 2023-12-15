#ifndef _LINE_H_
#define _LINE_H_
#include "Vec3.h"
#include "Helpers.h"
#include "Color.h"
#include "Matrix4.h"
#include <iostream>

class Line
{
	public:
		Vec3 p0;
		Vec3 p1;
		Color p0Color;
		Color p1Color;
	
		Line();
		Line(Vec3 p0, Vec3 p1, Color p0Color, Color p1Color);
		Line(const Line &other);
		Line &operator=(const Line &other);
		void applyTransformationMatrix(Matrix4& transformationMatrix);
		friend std::ostream &operator<<(std::ostream &os, const Line &l);

		Vec3 getPoint(double t);
};
#endif