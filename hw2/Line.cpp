#include "Line.h"

Line::Line()
{
	origin = Vec3();
	direction = Vec3();
}

Line::Line(Vec3 origin, Vec3 direction)
{
	this->origin = origin;
	this->direction = direction;
}

Line::Line(const Line &other)
{
	this->origin = other.origin;
	this->direction = other.direction;
}

Line &Line::operator=(const Line &other)
{
	this->origin = other.origin;
	this->direction = other.direction;
	return *this;
}

std::ostream &operator<<(std::ostream &os, const Line &l)
{
	os << "origin: " << l.origin << " direction: " << l.direction;
	return os;
}

Vec3 Line::getPoint(double t)
{
	return origin + direction * t;
}