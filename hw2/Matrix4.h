#ifndef __MATRIX4_H__
#define __MATRIX4_H__
#include <iostream>

class Matrix4
{
public:
    double values[4][4];

    Matrix4();
    Matrix4(double values[4][4]);
    Matrix4(const Matrix4 &other);
    Matrix4 transpose();
    friend std::ostream &operator<<(std::ostream &os, const Matrix4 &m);

    Matrix4 operator*(const Matrix4 &other);
    Matrix4 operator*= (const Matrix4 &other);
};

#endif