#include <iomanip>
#include "Vec4.h"

Vec4::Vec4()
{
    this->x = 0.0;
    this->y = 0.0;
    this->z = 0.0;
    this->t = 0.0;
    this->colorId = NO_COLOR;
}

Vec4::Vec4(double x, double y, double z, double t)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->t = t;
    this->colorId = NO_COLOR;
}

Vec4::Vec4(double x, double y, double z, double t, int colorId)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->t = t;
    this->colorId = colorId;
}

Vec4::Vec4(const Vec4 &other)
{
    this->x = other.x;
    this->y = other.y;
    this->z = other.z;
    this->t = other.t;
    this->colorId = other.colorId;
}

Vec4::Vec4(const Vec3& other)
{
    this->x = other.x;
    this->y = other.y;
    this->z = other.z;
    this->t = 1.0;
    this->colorId = other.colorId;
}

double Vec4::getNthComponent(int n)
{
    switch (n)
    {
    case 0:
        return this->x;

    case 1:
        return this->y;

    case 2:
        return this->z;

    case 3:
    default:
        return this->t;
    }
}

std::ostream &operator<<(std::ostream &os, const Vec4 &v)
{
    os << std::fixed << std::setprecision(6) << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.t << "]";
    return os;
}

Vec4 Vec4::operator/(double d)
{
    return Vec4(this->x / d, this->y / d, this->z / d, this->t / d, this->colorId);
}

Vec4 Vec4::operator*(Matrix4& m)
{
    double result[4];
    double total;

    for (int i = 0; i < 4; i++)
    {
        total = 0;
        for (int j = 0; j < 4; j++)
        {
            total += m.values[i][j] * this->getNthComponent(j);
        }
        result[i] = total;
    }
    return Vec4(result[0], result[1], result[2], result[3], this->colorId);
}

Vec4 Vec4::operator-(Vec4& v)
{
    return Vec4(this->x - v.x, this->y - v.y, this->z - v.z, this->t - v.t, this->colorId);
}