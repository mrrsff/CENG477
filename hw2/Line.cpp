#include "Line.h"

Line::Line()
{
	p0 = Vec3();
	p1 = Vec3();
}

Line::Line(Vec3 p0, Vec3 p1)
{
	this->p0 = p0;
	this->p1 = p1;
}

Line::Line(const Line &other)
{
	this->p0 = other.p0;
	this->p1 = other.p1;
}

Line &Line::operator=(const Line &other)
{
	this->p0 = other.p0;
	this->p1 = other.p1;
	return *this;	
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