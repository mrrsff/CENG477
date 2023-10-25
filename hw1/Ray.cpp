#include "Ray.h"
#include "parser.h"
using namespace parser;

Ray::Ray()
{
    this->origin = Vec3f{0,0,0};
    this->direction = Vec3f{0, 0, 0};
}

Ray::Ray(Vec3f origin, Vec3f direction)
{
    this->origin = origin;
    this->direction = direction;
}

Ray::~Ray()
{
}

Vec3f Ray::getDirection()
{
    return this->direction;
}

Vec3f Ray::getOrigin()
{
    return this->origin;
}

Vec3f Ray::getPoint(float t)
{
    return this->origin + this->direction * t;
}