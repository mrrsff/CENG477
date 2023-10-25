#pragma once //redefinition error cancel

#include "parser.h"
using namespace parser;

class Ray
{
    private:
        Vec3f origin;
        Vec3f direction;
    public:
        Ray();
        Ray(Vec3f origin, Vec3f direction);
        ~Ray();
        Vec3f getOrigin();
        Vec3f getDirection();
        Vec3f getPoint(float t);
};

