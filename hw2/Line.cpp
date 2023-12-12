#include "Line.h"

Line::Line()
{
	start = Vec3();
	end = Vec3();
}

Line::Line(Vec3 start = Vec3(), Vec3 end = Vec3())
{
	this->start = start;
	this->end = end;
}

Line::Line(const Line &other)
{
	this->start = other.start;
	this->end = other.end;
}

Line &Line::operator=(const Line &other)
{
	this->start = other.start;
	this->end = other.end;
	return *this;	
}

std::ostream &operator<<(std::ostream &os, const Line &l)
{
	os << "Line: " << l.start << " " << l.end;
	return os;
}

Vec3 Line::getPoint(double t)
{
	return start + (end - start) * t;
}