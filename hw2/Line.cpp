#include "Line.h"
#include "Vec4.h"

Line::Line()
{
	p0 = Vec3();
	p1 = Vec3();
	p0Color = Color();
	p1Color = Color();
}

Line::Line(Vec3 p0, Vec3 p1, Color p0Color, Color p1Color)
{
	this->p0 = p0;
	this->p1 = p1;
	this->p0Color = p0Color;
	this->p1Color = p1Color;
}

Line::Line(const Line &other)
{
	this->p0 = other.p0;
	this->p1 = other.p1;
	this->p0Color = other.p0Color;
	this->p1Color = other.p1Color;
}

Line &Line::operator=(const Line &other)
{
	this->p0 = other.p0;
	this->p1 = other.p1;
	this->p0Color = other.p0Color;
	this->p1Color = other.p1Color;
	return *this;	
}
void Line::applyTransformationMatrix(Matrix4& transformationMatrix)
{
	Vec4 p0Vec4 = Vec4(p0.x, p0.y, p0.z, 1);
	Vec4 p1Vec4 = Vec4(p1.x, p1.y, p1.z, 1);
	p0Vec4 = p0Vec4 * transformationMatrix;
	p1Vec4 = p1Vec4 * transformationMatrix;
	p0Vec4 = p0Vec4 / p0Vec4.t;
	p1Vec4 = p1Vec4 / p1Vec4.t;
	p0 = Vec3(p0Vec4.x, p0Vec4.y, p0Vec4.z, p0.colorId);
	p1 = Vec3(p1Vec4.x, p1Vec4.y, p1Vec4.z, p1.colorId);
}

std::ostream &operator<<(std::ostream &os, const Line &l)
{
	os << "Line: " << l.p0 << " " << l.p1;
	return os;
}

Vec3 Line::getPoint(double t)
{
	return p0 + (p1 - p0) * t;
}