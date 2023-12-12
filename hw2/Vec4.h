#ifndef __VEC4_H__
#define __VEC4_H__
#define NO_COLOR -1
#include <iostream>
#include "Vec3.h"
#include "Matrix4.h"

class Vec4
{
public:
    double x, y, z, t;
    int colorId;

    Vec4();
    Vec4(double x, double y, double z, double t);
    Vec4(double x, double y, double z, double t, int colorId);
    Vec4(const Vec4 &other);
    Vec4(const Vec3 &other);

    double getNthComponent(int n);

    friend std::ostream &operator<<(std::ostream &os, const Vec4 &v);

    Vec4 operator/(double d);
    Vec4 operator*(Matrix4& m);
};

#endif