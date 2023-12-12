#include <iomanip>
#include "Scaling.h"
#include "Helpers.h"
#include "Matrix4.h"
#include <cmath>

Scaling::Scaling() {
    this->scalingId = -1;
    this->sx = 0;
    this->sy = 0;
    this->sz = 0;
}


Matrix4 getIdentityMatrix();

Matrix4 getIdentityMatrix();
Scaling::Scaling(int scalingId, double sx, double sy, double sz)
{
    this->scalingId = scalingId;
    this->sx = sx;
    this->sy = sy;
    this->sz = sz;
}
Matrix4 Scaling::getScalingMatrix()
{
    Matrix4 scalingMatrix = getIdentityMatrix();
    scalingMatrix.values[0][0] = sx;
    scalingMatrix.values[1][1] = sy;
    scalingMatrix.values[2][2] = sz;

    return scalingMatrix;
}

std::ostream &operator<<(std::ostream &os, const Scaling &s)
{
    os << std::fixed << std::setprecision(3) << "Scaling " << s.scalingId << " => [" << s.sx << ", " << s.sy << ", " << s.sz << "]";

    return os;
}
